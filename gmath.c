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
  double theta = camera.fov * 0.5;

  vertex top = {0.0, 1.0, 0.0};
  vertex bottom = {0.0, -1.0, 0.0};
  vertex right = {1.0, 0.0, 0.0};
  vertex left = {-1.0, 0.0, 0.0};
  vertex near = {0.0, 0.0, -1.0};
  vertex far = {0.0, 0.0, 1.0};

  camera.frustum[0] = rotate_x (top, theta);
  camera.frustum[1] = rotate_x (bottom, -theta);
  camera.frustum[2] = rotate_y (right, -theta);
  camera.frustum[3] = rotate_y (left, theta);
  camera.frustum[4] = near;
  camera.frustum[5] = far;
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
  vertex n = norm (cross (u, v));
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
      if (i < 4)
        {
          vertex closest;

          closest.x = camera.frustum[i].x < 0.0 ? min.x : max.x;
          closest.y = camera.frustum[i].y < 0.0 ? min.y : max.y;
          closest.z = camera.frustum[i].z < 0.0 ? min.z : max.z;

          if (dot (camera.frustum[i], closest) < 0.0)
            return 1;
        }
      else if (i == 4 && min.z < camera.near)
        return 1;
      else if (i == 5 && min.z > camera.far)
        return 1;
    }

  return 0;
}
