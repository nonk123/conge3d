#ifndef MESH_H
#define MESH_H

#include <stdio.h>

#include "conge/conge.h"

#include "vmath.h"

typedef struct mesh_t mesh_t;
struct mesh_t
{
  vertex* vertices;
  v_index vertex_count;
  v_index* indices;
  v_index index_count;
  vertex aabb[2]; /* the "base" AABB, without transformations */
};

typedef struct mesh_instance mesh_instance;
struct mesh_instance
{
  mesh_t* mesh;
  vertex position;
  vertex rotation; /* in Euler angles */
};

/*
 * Properly free an instance of mesh_t. Do nothing if it's null.
 */
void free_mesh (mesh_t*);

/*
 * Draw a mesh instance on the screen, with MVP applied.
 */
void draw_mesh_instance (conge_ctx*, mesh_instance);

/*
 * Apply model transform (matrix) onto a given vertex.
 */
vertex apply_model (mesh_instance, vertex);

/*
 * Load a mesh from the given .obj file.
 *
 * No checks are done for a malformed .obj.
 */
mesh_t* load_obj (FILE*);

#endif /* MESH_H */
