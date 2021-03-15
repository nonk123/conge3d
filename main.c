#include "conge/conge.h"
#include <math.h>

// clang-format off

#define CLAMP(V, MIN, MAX) (CONGE_MIN (CONGE_MAX ((V), MIN), MAX))

typedef struct vertex vertex;
struct vertex { double x, y, z; };

typedef short index_t;

static vertex cube[8] = {
  {0.0, 0.0, 0.0},
  {1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, 0.0, 1.0},
  {1.0, 1.0, 0.0},
  {0.0, 1.0, 1.0},
  {1.0, 0.0, 1.0},
  {1.0, 1.0, 1.0},
};

/* Three vertices make a triangle. */
static index_t indices[36] = {
  0, 5, 2,  0, 3, 5,  1, 4, 7,  1, 7, 6, /* left and right */
  0, 2, 4,  0, 4, 1,  3, 7, 5,  3, 6, 7, /* front and back */
  0, 6, 3,  0, 1, 6,  2, 5, 7,  2, 7, 4, /* bottom and top */
};

vertex
add (vertex a, vertex b)
{
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;

  return a;
}

vertex
sub (vertex a, vertex b)
{
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;

  return a;
}

vertex
mult (vertex v, double s)
{
  v.x *= s;
  v.y *= s;
  v.z *= s;

  return v;
}

/*
 * Return the length of vector V.
 */
double
length (vertex v)
{
  return sqrt (pow (v.x, 2.0) + pow (v.y, 2.0) + pow (v.z, 2.0));
}

/*
 * Normalize the vector V by dividing it by its length.
 */
vertex
norm (vertex v)
{
  return mult (v, 1.0 / length (v));
}

/*
 * Return the distance between vectors A and B.
 */
double
dist (vertex a, vertex b)
{
  return length (sub (b, a));
}

/*
 * Calculate the cross product of vectors A and B.
 */
vertex
cross (vertex a, vertex b)
{
  vertex c;

  c.x = a.y * b.z - a.z * b.y;
  c.y = a.z * b.x - a.x * b.z;
  c.z = a.x * b.y - a.y * b.x;

  return c;
}

/*
 * Return the dot product of vectors A and B.
 */
double
dot (vertex a, vertex b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

/*
 * Rotate V around the X axis by given THETA.
 */
vertex
rotate_x (vertex v, double theta)
{
  vertex r = {v.x, 0.0, 0.0};
  r.y = v.y * cos (theta) - v.z * sin (theta);
  r.z = v.y * sin (theta) + v.z * cos (theta);
  return r;
}

/*
 * Rotate V around the Y axis by given THETA.
 */
vertex
rotate_y (vertex v, double theta)
{
  vertex r = {0.0, v.y, 0.0};
  r.x = v.x * cos (theta) + v.z * sin (theta);
  r.z = v.z * cos (theta) - v.x * sin (theta);
  return r;
}

/*
 * Rotate V around the Z axis by given THETA.
 */
vertex
rotate_z (vertex v, double theta)
{
  vertex r = {0.0, 0.0, v.z};
  r.x = v.x * cos (theta) - v.y * sin (theta);
  r.y = v.x * sin (theta) + v.y * cos (theta);
  return r;
}

static struct
{
  vertex position;
  vertex rotation; /* in euler angles */
  double fov, n, f; /* near and far are already macros, wtf? */
  vertex frustum[6]; /* frustum normals */
} camera = {
  {0.0, 0.0, -2.0},
  {0.0, 0.0, 0.0},
  M_PI_2,
  0.05,
  5.0,
};

/*
 * Called every time the aspect ratio changes.
 */
void
compute_frustum (double ar)
{
  /* Half viewport size. */
  double hh = tan (camera.fov * 0.5);
  double hw = hh / ar;

  /* Viewport corners. */
  vertex nw = {-hw,  hh, 1.0};
  vertex ne = { hw,  hh, 1.0};
  vertex sw = {-hw, -hh, 1.0};
  vertex se = { hw, -hh, 1.0};

  /* Frustum planes normals. */
  vertex top = norm (cross (nw, ne));
  vertex bottom = norm (cross (se, sw));
  vertex right = norm (cross (ne, se));
  vertex left = norm (cross (sw, nw));
  vertex n = {0.0, 0.0, 1.0};
  vertex f = {0.0, 0.0, -1.0};

  camera.frustum[0] = top;
  camera.frustum[1] = bottom;
  camera.frustum[2] = left;
  camera.frustum[3] = right;
  camera.frustum[4] = n;
  camera.frustum[5] = f;
}

/*
 * Convert V to screen coordinates.
 */
vertex
to_screen (conge_ctx* ctx, vertex v)
{
  v.x = round ((v.x + 1.0) * 0.5 * ctx->cols);
  v.y = round ((1.0 - v.y) * 0.5 * ctx->rows);

  return v;
}

/*
 * Apply inverse camera transformations to vector V.
 */
vertex
view (vertex v)
{
  v = sub (v, camera.position);

  v = rotate_y (v, -camera.rotation.y);
  v = rotate_x (v, -camera.rotation.x);
  v = rotate_z (v, -camera.rotation.z);

  return v;
}

/*
 * Project vector V onto the screen space. AR is the aspect ratio.
 */
vertex
project (vertex v, double ar)
{
  double f = 1.0 / tan (0.5 * camera.fov);
  double q = camera.f / (camera.f - camera.n);

  vertex r;

  r.x = v.x * f * ar;
  r.y = v.y * f;
  r.z = v.z * q - camera.n * q;

  if (fabs (v.z) > 0.01)
    {
      r.x /= v.z;
      r.y /= v.z;
    }

  return r;
}

/*
 * Calculate the normal of a triangle surface defined by its vertices.
 */
vertex
calculate_normal (vertex a, vertex b, vertex c)
{
  vertex u = sub (b, a), v = sub (c, a);
  vertex n = cross (u, v);

  double l = length (n);

  n.x /= l;
  n.y /= l;
  n.z /= l;

  return n;
}

/*
 * Return true if the AABB should be culled.
 */
int
cull (vertex v1, vertex v2)
{
  int i = 0;

  for (; i < 6; i++)
    {
      vertex furthest;

      furthest.x = camera.frustum[i].x > 0.0 ? v1.x : v2.x;
      furthest.y = camera.frustum[i].y > 0.0 ? v1.y : v2.y;
      furthest.z = camera.frustum[i].z > 0.0 ? v1.z : v2.z;

      if (i < 4 && dot (camera.frustum[i], furthest) > 0.0)
        return 1;

      if (i == 4 && furthest.z < camera.n)
        return 1;

      if (i == 5 && furthest.z > camera.f)
        return 1;
    }

  return 0;
}

void
tick (conge_ctx* ctx)
{
  int x, y, i;

  vertex vertices[8];

  /* Multiply by two to compensate for the weird font size. */
  double new_ar = 2.0 * ctx->rows / ctx->cols;
  static double ar = 0.0;

  vertex movement = {0.0, 0.0, 0.0};
  vertex rotation = {0.0, 0.0, 0.0};

  double movement_speed = 1.0;
  double rotation_speed = M_PI_2 * ctx->delta;

  vertex v1, v2; /* cube AABB */

  if (new_ar != ar)
    {
      ar = new_ar;
      compute_frustum (ar);
    }

  strcpy (ctx->title, "conge3d");

  if (ctx->keys[CONGE_ESC])
    {
      ctx->exit = 1;
      return;
    }

  /* Camera controls. */

  movement.x = ctx->keys[CONGE_D] - ctx->keys[CONGE_A];
  movement.y = ctx->keys[CONGE_E] - ctx->keys[CONGE_Q];
  movement.z = ctx->keys[CONGE_W] - ctx->keys[CONGE_S];
  movement = rotate_y (movement, camera.rotation.y);
  movement = mult (movement, movement_speed * ctx->delta);

  camera.position = add (camera.position, movement);

  rotation.y = ctx->keys[CONGE_L] - ctx->keys[CONGE_J];
  rotation.x = CLAMP (ctx->keys[CONGE_K] - ctx->keys[CONGE_I], -M_PI_2, M_PI_2);
  rotation = rotate_y (rotation, rotation.y);
  rotation = mult (rotation, rotation_speed);

  camera.rotation = add (camera.rotation, rotation);

  /* FIXME: rotating both Y and X does weird things. */

  for (i = 0; i < 8; i++)
    vertices[i] = view (cube[i]);

  v1 = vertices[0];
  v2 = vertices[7];

  /* Calculate the AABB. */
  for (i = 0; i < 8; i++)
    {
      vertex v = vertices[i];

      v1.x = CONGE_MIN (v1.x, v.x);
      v1.y = CONGE_MIN (v1.y, v.y);
      v1.z = CONGE_MIN (v1.z, v.z);

      v2.x = CONGE_MAX (v2.x, v.x);
      v2.y = CONGE_MAX (v2.y, v.y);
      v2.z = CONGE_MAX (v2.z, v.z);

      /* Prepare the vertex for rendering. */
      vertices[i] = project (v, ar);
    }

  if (cull (v1, v2))
    return;

  for (i = 0; i < 36; i += 3)
    {
      /* Build a triangle out of vertices. */
      vertex a = to_screen (ctx, vertices[indices[i + 0]]);
      vertex b = to_screen (ctx, vertices[indices[i + 1]]);
      vertex c = to_screen (ctx, vertices[indices[i + 2]]);

      vertex n = calculate_normal (a, b, c);

      /* Only draw the visible faces. */
      if (n.z > 0.0)
        {
          conge_pixel fill = {' ', CONGE_BLACK, CONGE_WHITE};

          conge_draw_line (ctx, a.x, a.y, b.x, b.y, fill);
          conge_draw_line (ctx, b.x, b.y, c.x, c.y, fill);
          conge_draw_line (ctx, c.x, c.y, a.x, a.y, fill);
        }
    }
}

int
main (void)
{
  conge_ctx* ctx = conge_init ();

  if (ctx == NULL)
    return 1;

  conge_run (ctx, tick, 30);
  conge_free (ctx);

  return 0;
}

// clang-format on
