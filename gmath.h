/*
 * gmath.h - graphics-related math.
 *
 * It takes a lot of space and should rest in a separate file.
 */

#ifndef GMATH_H
#define GMATH_H

#include "conge/conge.h"

#include "vmath.h"

vertex get_camera_position (void);
void set_camera_position (vertex);

vertex get_camera_rotation (void);
void set_camera_rotation (vertex);

void translate_camera (vertex);
void rotate_camera (vertex);

void set_camera_fov (double);
void set_camera_near (double);
void set_camera_far (double);

/*
 * Must be called before a frame is rendered.
 *
 * This function updates the frustum planes and the screen projection, all in
 * order to match up with the current aspect ratio.
 */
void prepare_graphics (conge_ctx*);

/*
 * Calculate the normal of a triangle (surface) defined by its vertices.
 */
vertex tri_normal (vertex, vertex, vertex);

/*
 * Apply view transformations from the camera to the given vertex.
 */
vertex view (vertex);

/*
 * Project the given vertex to screen space.
 */
vertex project (vertex);

/*
 * Convert a vertex in screen space to screen coordinates.
 */
vertex norm_to_screen (vertex);

/*
 * Return true if the given AABB, defined by its two corners, is culled.
 */
int cull_aabb (vertex, vertex);

#endif /* GMATH_H */
