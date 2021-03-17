#include <stdio.h>
#include <stdlib.h>

#include "mesh.h"
#include "gmath.h"

void
free_mesh (mesh_t* mesh)
{
  if (mesh == NULL)
    return;

  if (mesh->vertices != NULL)
    {
      free (mesh->vertices);
      mesh->vertices = NULL;
    }

  if (mesh->indices != NULL)
    {
      free (mesh->indices);
      mesh->indices = NULL;
    }

  free (mesh);
  mesh = NULL;
}

void
draw_mesh_instance (conge_ctx* ctx, mesh_instance instance)
{
  int i = 0;

  vertex* transformed = NULL;
  vertex min, max;

  if (instance.mesh == NULL)
    return;

  transformed = calloc (instance.mesh->vertex_count, sizeof (vertex));

  for (i = 0; i < instance.mesh->vertex_count; i++)
    {
      transformed[i] = project (view (apply_model (instance, instance.mesh->vertices[i])));

      /* Find the minimum and the maximum point of the bounding box. */
      if (i == 0)
        min = max = transformed[i];
      else
        {
          min.x = MIN (min.x, transformed[i].x);
          min.y = MIN (min.y, transformed[i].y);
          min.z = MIN (min.z, transformed[i].z);

          max.x = MAX (max.x, transformed[i].x);
          max.y = MAX (max.y, transformed[i].y);
          max.z = MAX (max.z, transformed[i].z);
        }
    }

  if (cull_aabb (min, max))
    {
      free (transformed);
      return;
    }

  for (i = 0; i < instance.mesh->index_count; i += 3)
    {
      /* Build a triangle out of vertices. */
      vertex a = norm_to_screen (transformed[instance.mesh->indices[i + 2]]);
      vertex b = norm_to_screen (transformed[instance.mesh->indices[i + 1]]);
      vertex c = norm_to_screen (transformed[instance.mesh->indices[i + 0]]);

      vertex n = tri_normal (a, b, c);

      /* Only draw the visible faces. */
      if (n.z < 0.0)
        {
          conge_pixel fill = {' ', CONGE_BLACK, CONGE_WHITE};
          conge_draw_triangle (ctx, a.x, a.y, b.x, b.y, c.x, c.y, fill);
        }
    }

  free (transformed);
}

vertex
apply_model (mesh_instance instance, vertex v)
{
  v = rotate_x (v, instance.rotation.x);
  v = rotate_y (v, instance.rotation.y);
  v = rotate_z (v, instance.rotation.z);

  v = add (v, instance.position);

  return v;
}

#define READ_LINE (fgets (line, 256, fh) != NULL)

mesh_t* load_obj (FILE* fh)
{
  char line[256];

  int vertex_count = 0, index_count = 0;
  int idx_per_face = 3; /* 3 for triangles and 4 for quads */

  mesh_t* mesh = malloc (sizeof *mesh);

  /* Count the vertices and indices. */
  while (READ_LINE)
    {
      if (line[0] == 'v' && line[1] == ' ')
        vertex_count++;

      if (line[0] == 'f' && line[1] == ' ')
        {
          int tmp;

          char* s = line;
          int occurences = 0;

          /* Detect quad faces by looking for an extra set of slashes. */
          if (index_count == 0)
            {
              while ((s = strstr (s, "/")) != NULL)
                {
                  occurences++;
                  s++;
                }

              if (occurences == 8)
                idx_per_face = 4;
            }

          index_count += idx_per_face == 3 ? 3 : 6;
        }
    }

  mesh->vertices = calloc (vertex_count, sizeof (vertex));
  mesh->vertex_count = vertex_count;

  mesh->indices = calloc (index_count, sizeof (v_index));
  mesh->index_count = index_count;

  fseek (fh, 0, SEEK_SET);

  vertex_count = 0;
  index_count = 0;

  /* Fill in the vertices and indices. */
  while (READ_LINE)
    {
      if (line[0] == 'v' && line[1] == ' ')
        {
          vertex* v = &mesh->vertices[vertex_count++];
          sscanf(line, "v %lf %lf %lf", &v->x, &v->y, &v->z);
        }

      if (line[0] == 'f' && line[1] == ' ')
        {
          char* s = &line[1];
          v_index indices[4];
          int j;

          for (j = 0; j < idx_per_face; j++)
            {
              sscanf (s, " %hd/", &indices[j]);
              indices[j]--; /* .obj indices start with 1 */

              /* Skip until next space. */
              while (*++s != ' ' && *s != '\n');
            }

          /* Normals and textures are ignored for now. */
          if (idx_per_face == 3)
            {
              mesh->indices[index_count++] = indices[0];
              mesh->indices[index_count++] = indices[1];
              mesh->indices[index_count++] = indices[2];
            }
          else
            {
              mesh->indices[index_count++] = indices[0];
              mesh->indices[index_count++] = indices[1];
              mesh->indices[index_count++] = indices[2];
              mesh->indices[index_count++] = indices[0];
              mesh->indices[index_count++] = indices[2];
              mesh->indices[index_count++] = indices[3];
            }
        }
    }

  return mesh;
}

#undef READ_LINE
