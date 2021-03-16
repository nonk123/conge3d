#include <math.h>

#include "gmath.h"

/* Used internally for projection calculations. */
static double aspect_ratio = 1.0;

/* Screen dimensions received from the current frame. */
static double s_width, s_height;

/* The one and only camera. */
static struct
{
  vertex position;
  vertex rotation; /* in Euler angles */
  double fov, near, far;
  vertex frustum[6]; /* frustum normals */
} camera = {0};

vertex
get_camera_position ()
{
  return camera.position;
}

void
set_camera_position (vertex p)
{
  camera.position = p;
}

vertex
get_camera_rotation ()
{
  return camera.rotation;
}

void
set_camera_rotation (vertex r)
{
  camera.rotation = r;
}

void
translate_camera (vertex d)
{
  camera.position = add (camera.position, d);
}

void
rotate_camera (vertex e)
{
  camera.rotation = add (camera.rotation, e);
}

void
set_camera_fov (double fov)
{
  camera.fov = fov;
}

void
set_camera_near (double near)
{
  camera.near = near;
}

void
set_camera_far (double far)
{
  camera.far = far;
}

void
update_frustum ()
{
  /* Half viewport size. */
  double hh = tan (camera.fov * 0.5);
  double hw = hh / aspect_ratio;

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
  vertex n = {0.0, 0.0, -1.0};
  vertex f = {0.0, 0.0, 1.0};

  camera.frustum[0] = top;
  camera.frustum[1] = bottom;
  camera.frustum[2] = left;
  camera.frustum[3] = right;
  camera.frustum[4] = n;
  camera.frustum[5] = f;
}

void
prepare_graphics (conge_ctx* ctx)
{
  CONSOLE_FONT_INFO font_info;

  double console_font_ar;

  GetCurrentConsoleFont (ctx->internal.output, FALSE, &font_info);

  s_width = ctx->cols;
  s_height = ctx->rows;

  console_font_ar = (double) font_info.dwFontSize.Y / font_info.dwFontSize.X;
  aspect_ratio = console_font_ar * s_height / s_width;

  update_frustum ();
}

vertex
tri_normal (vertex a, vertex b, vertex c)
{
  vertex u = sub (b, a), v = sub (c, a);
  vertex n = cross (u, v);

  double l = length (n);

  n.x /= l;
  n.y /= l;
  n.z /= l;

  return n;
}

vertex
norm_to_screen (vertex v)
{
  /* Screen coordinates from 0.0 to 1.0 */
  double sx = (v.x + 1.0) * 0.5;
  double sy = (1.0 - v.y) * 0.5;

  sx = CLAMP (sx, 0.0, 1.0);
  sy = CLAMP (sy, 0.0, 1.0);

  v.x = round (sx * s_width);
  v.y = round (sy * s_height);

  return v;
}

vertex
view (vertex v)
{
  v = sub (v, camera.position);

  v = rotate_y (v, -camera.rotation.y);
  v = rotate_x (v, -camera.rotation.x);
  v = rotate_z (v, -camera.rotation.z);

  return v;
}

vertex
project (vertex v)
{
  double f = 1.0 / tan (0.5 * camera.fov);
  double q = camera.far / (camera.far - camera.near);

  vertex r;

  /* This is basically a black box for me. */
  r.x = v.x * f * aspect_ratio;
  r.y = v.y * f;
  r.z = (v.z - camera.near) * q;

  if (fabs (v.z) > 0.0001)
    {
      r.x /= v.z;
      r.y /= v.z;
    }

  return r;
}

int
cull_aabb (vertex min, vertex max)
{
  int i = 0;

  for (; i < 6; i++)
    {
      vertex furthest;

      furthest.x = camera.frustum[i].x < 0.0 ? min.x : max.x;
      furthest.y = camera.frustum[i].y < 0.0 ? min.y : max.y;
      furthest.z = camera.frustum[i].z < 0.0 ? min.z : max.z;

      if (i < 4 && dot (camera.frustum[i], furthest) > 0.0)
        return 1;

      if (i == 4 && furthest.z < camera.near)
        return 1;

      if (i == 5 && furthest.z > camera.far)
        return 1;
    }

  return 0;
}
