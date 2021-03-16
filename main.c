/*
 * main.c - the entry point.
 *
 * This file initializes the conge library, and handles the rendering of things.
 */

#include "gmath.h"
#include "mesh.h"

#include "conge/conge.h"

static mesh_instance teapots[10];

void
control_camera (conge_ctx* ctx)
{
  vertex movement = {0.0, 0.0, 0.0};
  double movement_speed = 1.0 * ctx->delta;

  double mouse_sensitivity = 0.05;

  static int grab_key_pressed_last_frame = 0;

  movement.x = ctx->keys[CONGE_D] - ctx->keys[CONGE_A];
  movement.z = ctx->keys[CONGE_W] - ctx->keys[CONGE_S];
  movement.y = ctx->keys[CONGE_SPACEBAR] - ctx->keys[CONGE_LCTRL];

  /* Align horizonal movement with the camera. */
  movement = rotate_y (movement, get_camera_rotation ().y);
  movement = mult (movement, movement_speed);
  translate_camera (movement);

  if (ctx->grab)
    {
      vertex dr = {0.0, 0.0, 0.0}, rotation = get_camera_rotation ();
      double rotation_speed = M_PI_2 * ctx->delta;

      dr.x = ctx->mouse_dy * mouse_sensitivity;
      dr.y = ctx->mouse_dx * mouse_sensitivity;

      rotation = add (rotation, mult (dr, rotation_speed));
      /* Prevent making frontflips/backflips with the camera. */
      rotation.x = CLAMP (rotation.x, -M_PI_2, M_PI_2);

      set_camera_rotation (rotation);
    }

  /* Emulate "key just pressed" event. */
  if (ctx->keys[CONGE_LALT])
    {
      if (!grab_key_pressed_last_frame)
        ctx->grab = !ctx->grab;

      grab_key_pressed_last_frame = 1;
    }
  else
    grab_key_pressed_last_frame = 0;
}

void
draw_mesh_instance (conge_ctx* ctx, mesh_instance instance)
{
  int i;

  if (cull_mesh_instance (instance))
    return;

  for (i = 0; i < instance.mesh->index_count; i += 3)
    {
      /* Build a triangle out of vertices. */
      vertex a = instance.mesh->vertices[instance.mesh->indices[i + 0]];
      vertex b = instance.mesh->vertices[instance.mesh->indices[i + 1]];
      vertex c = instance.mesh->vertices[instance.mesh->indices[i + 2]];

      vertex n;

      a = norm_to_screen (project (view (apply_model (instance, a))));
      b = norm_to_screen (project (view (apply_model (instance, b))));
      c = norm_to_screen (project (view (apply_model (instance, c))));

      n = tri_normal (a, b, c);

      /* Only draw the visible faces. */
      if (n.z > 0.0)
        {
          conge_pixel fill = {' ', CONGE_BLACK, CONGE_WHITE};
          conge_draw_triangle (ctx, c.x, c.y, b.x, b.y, a.x, a.y, fill);
        }
    }
}

void
tick (conge_ctx* ctx)
{
  int i;

  strcpy (ctx->title, "conge3d");

  if (ctx->keys[CONGE_ESC])
    {
      ctx->exit = 1;
      return;
    }

  control_camera (ctx);
  prepare_graphics (ctx);

  for (i = 0; i < 10; i++)
    draw_mesh_instance (ctx, teapots[i]);
}

int
main (void)
{
  conge_ctx* ctx = conge_init ();

  vertex c_pos = {0.0, 0.0, -10.0};
  vertex zero = {0.0, 0.0, 0.0};

  FILE* fh = fopen ("teapot.obj", "r");
  mesh_t* mesh = NULL;

  int i;

  if (ctx == NULL)
    return 1;

  if (fh == NULL)
    return 2;

  mesh = load_obj (fh);

  for (i = 0; i < 10; i++)
    {
      vertex offset = {8.0 * (i - 5.0), 0.0, 0.0};

      teapots[i].mesh = mesh;
      teapots[i].position = add (zero, offset);
      teapots[i].rotation = zero;
    }

  set_camera_position (c_pos);
  set_camera_rotation (zero);
  set_camera_fov (M_PI_2);
  set_camera_near (0.01);
  set_camera_far (100.0);

  ctx->grab = 1;

  conge_run (ctx, tick, 30);
  conge_free (ctx);

  free_mesh (mesh);

  return 0;
}
