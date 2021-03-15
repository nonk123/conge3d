/*
 * main.c - the entry point.
 *
 * This file initializes the conge library, and handles the rendering of things.
 */

#include "gmath.h"

#include "conge/conge.h"

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
static v_index indices[36] = {
  0, 5, 2,  0, 3, 5,  1, 4, 7,  1, 7, 6, /* left and right */
  0, 2, 4,  0, 4, 1,  3, 7, 5,  3, 6, 7, /* front and back */
  0, 6, 3,  0, 1, 6,  2, 5, 7,  2, 7, 4, /* bottom and top */
};

void
tick (conge_ctx* ctx)
{
  int x, y, i;

  vertex vertices[8];

  vertex movement = {0.0, 0.0, 0.0};
  vertex rotation = {0.0, 0.0, 0.0};

  double movement_speed = 1.0;
  double rotation_speed = M_PI_2 * ctx->delta;

  vertex v1, v2; /* cube AABB */

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

  /* Align horizonal movement with the camera. */
  movement = rotate_y (movement, get_camera_rotation().y);
  movement = mult (movement, movement_speed * ctx->delta);
  translate_camera (movement);

  rotation.x = CLAMP (ctx->keys[CONGE_K] - ctx->keys[CONGE_I], -M_PI_2, M_PI_2);
  rotation = rotate_y (rotation, rotation.y);
  rotation.y = ctx->keys[CONGE_L] - ctx->keys[CONGE_J];
  rotation = mult (rotation, rotation_speed);
  rotate_camera (rotation);

  prepare_graphics (ctx->cols, ctx->rows);

  for (i = 0; i < 8; i++)
    vertices[i] = view (cube[i]);

  v1 = vertices[0];
  v2 = vertices[7];

  /* Calculate the AABB. */
  for (i = 0; i < 8; i++)
    {
      vertex v = vertices[i];

      v1.x = MIN (v1.x, v.x);
      v1.y = MIN (v1.y, v.y);
      v1.z = MIN (v1.z, v.z);

      v2.x = MAX (v2.x, v.x);
      v2.y = MAX (v2.y, v.y);
      v2.z = MAX (v2.z, v.z);

      /* Prepare the vertex for rendering. */
      vertices[i] = project (v);
    }

  if (cull (v1, v2))
    return;

  for (i = 0; i < 36; i += 3)
    {
      /* Build a triangle out of vertices. */
      vertex a = norm_to_screen (vertices[indices[i + 0]]);
      vertex b = norm_to_screen (vertices[indices[i + 1]]);
      vertex c = norm_to_screen (vertices[indices[i + 2]]);

      vertex n = tri_normal (a, b, c);

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

  vertex c_pos = {0.0, 0.0, -2.0};
  vertex c_rot = {0.0, 0.0, 0.0};

  if (ctx == NULL)
    return 1;

  set_camera_position (c_pos);
  set_camera_rotation (c_rot);
  set_camera_fov (M_PI_2);
  set_camera_near (0.05);
  set_camera_far (100.0);

  conge_run (ctx, tick, 30);
  conge_free (ctx);

  return 0;
}
