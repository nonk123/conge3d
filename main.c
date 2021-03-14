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

#define INDICES_COUNT 36

/* Three vertices make a triangle. */
static index_t indices[INDICES_COUNT] = {
  0, 2, 4,  0, 4, 1,  1, 4, 7,  1, 7, 6,
  0, 2, 5,  0, 5, 3,  3, 5, 7,  3, 7, 6,
  0, 3, 6,  0, 6, 1,  2, 5, 7,  2, 7, 4,
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

static struct
{
  vertex position;
  vertex rotation; /* in euler angles */
  double fov, n, f; /* near and far are already macros, wtf? */
} camera = {
  {0.0, 0.0, -0.5},
  {0.0, 0.0, 0.0},
  M_PI_2,
  1.0,
  1000.0,
};

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

/*
 * Convert V to screen coordinates.
 */
vertex
to_screen (conge_ctx* ctx, vertex v)
{
  v.x = round ((v.x + 1.0) * 0.5 * ctx->cols);
  v.y = round ((v.y + 1.0) * 0.5 * ctx->rows);

  return v;
}

/*
 * Project V onto the screen space, with camera transformation applied.
 *
 * AR is the aspect ratio.
 */
vertex
project (vertex v, double ar)
{
  double f = 1.0 / tan (0.5 * camera.fov);
  double q = camera.f / (camera.f - camera.n);

  vertex r;

  v = sub (v, camera.position);

  v = rotate_y (v, -camera.rotation.y);
  v = rotate_x (v, -camera.rotation.x);
  v = rotate_z (v, -camera.rotation.z);

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

void
tick (conge_ctx* ctx)
{
  int x, y, i;

  /* Multiply by two to compensate for the weird font size. */
  double ar = 2.0 * ctx->rows / ctx->cols;

  vertex movement = {0.0, 0.0, 0.0};
  vertex rotation = {0.0, 0.0, 0.0};

  double movement_speed = 1.0;
  double rotation_speed = M_PI_2 * ctx->delta;

  strcpy (ctx->title, "conge3d");

  if (ctx->keys[CONGE_ESC])
    {
      ctx->exit = 1;
      return;
    }

  /* Camera controls. */

  movement.x = ctx->keys[CONGE_D] - ctx->keys[CONGE_A];
  movement.y = ctx->keys[CONGE_Q] - ctx->keys[CONGE_E];
  movement.z = ctx->keys[CONGE_W] - ctx->keys[CONGE_S];
  movement = rotate_y (movement, camera.rotation.y);
  movement = mult (movement, movement_speed * ctx->delta);

  camera.position = add (camera.position, movement);

  rotation.y = ctx->keys[CONGE_L] - ctx->keys[CONGE_J];
  rotation.x = CLAMP (ctx->keys[CONGE_I] - ctx->keys[CONGE_K], -M_PI_2, M_PI_2);
  rotation = rotate_y (rotation, rotation.y);
  rotation = mult (rotation, rotation_speed);

  camera.rotation = add (camera.rotation, rotation);

  /* FIXME: rotating both Y and X does weird things. */

  for (i = 0; i < INDICES_COUNT; i += 3)
    {
      /* Build a triangle out of vertices. */
      vertex a = to_screen (ctx, project (cube[indices[i + 0]], ar));
      vertex b = to_screen (ctx, project (cube[indices[i + 1]], ar));
      vertex c = to_screen (ctx, project (cube[indices[i + 2]], ar));

      conge_pixel fill = {' ', CONGE_BLACK, CONGE_WHITE};

      conge_draw_line (ctx, a.x, a.y, b.x, b.y, fill);
      conge_draw_line (ctx, b.x, b.y, c.x, c.y, fill);
      conge_draw_line (ctx, c.x, c.y, a.x, a.y, fill);
    }
}

int
main (void)
{
  conge_ctx* ctx = conge_init ();
  conge_run (ctx, tick, 30);
  conge_free (ctx);

  return 0;
}

// clang-format on
