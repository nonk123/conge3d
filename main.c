/*
 * main.c - the entry point.
 *
 * This file initializes the conge library, and handles the rendering of things.
 */

#include "gmath.h"
#include "mesh.h"

#include "conge/conge.h"

static mesh_instance model;

void
control_camera (conge_ctx* ctx)
{
  vertex movement = {0.0, 0.0, 0.0};
  double movement_speed = 1.0 * ctx->delta;

  double mouse_sensitivity = 0.02;

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
tick (conge_ctx* ctx)
{
  int i;
  char fps[64];

  strcpy (ctx->title, "conge3d");
  sprintf (fps, "FPS: %d", ctx->fps);

  if (ctx->keys[CONGE_ESC])
    {
      ctx->exit = 1;
      return;
    }

  control_camera (ctx);
  prepare_graphics (ctx);

  draw_mesh_instance (ctx, model);
  model.rotation.y -= M_PI_2 * ctx->delta;

  conge_write_string (ctx, fps, 0, 0, CONGE_WHITE, CONGE_BLACK);
}

int
main (void)
{
  conge_ctx* ctx = conge_init ();

  vertex c_pos = {0.0, 0.5, -1.0};
  vertex zero = {0.0, 0.0, 0.0};

  FILE* fh = fopen ("amogus.obj", "r");
  mesh_t* mesh = NULL;

  int i;

  if (ctx == NULL)
    return 1;

  if (fh == NULL)
    return 2;

  mesh = load_obj (fh);

  model.mesh = mesh;
  model.position = zero;
  model.rotation = zero;
  model.rotation.y = M_PI;

  set_camera_position (c_pos);
  set_camera_rotation (zero);
  set_camera_fov (M_PI_2);
  set_camera_near (0.1);
  set_camera_far (100.0);

  ctx->grab = 1;

  conge_run (ctx, tick, 60);
  conge_free (ctx);

  free_mesh (mesh);

  return 0;
}
