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

vertex
apply_model (mesh_instance instance, vertex v)
{
  v = add (v, instance.position);

  v = rotate_x (v, instance.rotation.x);
  v = rotate_y (v, instance.rotation.y);
  v = rotate_z (v, instance.rotation.z);

  return v;
}

int
cull_mesh_instance (mesh_instance instance)
{
  vertex min, max;
  int i;

  if (instance.mesh == NULL)
    return 1;

  /* Construct an AABB from the mesh. */
  for (i = 0; i < instance.mesh->vertex_count; i++)
    {
      vertex v = instance.mesh->vertices[i];
      v = project (view (apply_model (instance, v)));

      if (i == 0)
        min = max = v;
      else
        {
          min.x = MIN (min.x, v.x);
          min.y = MIN (min.y, v.y);
          min.z = MIN (min.z, v.z);

          max.x = MAX (max.x, v.x);
          max.y = MAX (max.y, v.y);
          max.z = MAX (max.z, v.z);
        }
    }

  return cull_aabb (min, max);
}

#define READ_LINE (fgets (line, 256, fh) != NULL)

mesh_t* load_obj (FILE* fh)
{
  char line[256];

  int vertex_count = 0, index_count = 0;

  mesh_t* mesh = malloc (sizeof *mesh);

  /* Count the vertices and indices. */
  while (READ_LINE)
    {
      if (line[0] == 'v' && line[1] == ' ')
        vertex_count++;

      if (line[0] == 'f' && line[1] == ' ')
        index_count += 3;
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
          v_index v1, t1, n1, v2, t2, n2, v3, t3, n3;

          /* Special handling when texture index is omitted. */
          if (strstr (line, "//") != NULL)
            sscanf (line, "f %hd//%hd %hd//%hd %hd//%hd",
                    &v1, &n1, &v2, &n2, &v3, &n3);
          else
            sscanf (line, "f %hd/%hd/%hd %hd/%hd/%hd %hd/%hd/%hd",
                    &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);

          /* Normals and textures are ignored for now. */
          mesh->indices[index_count++] = v1;
          mesh->indices[index_count++] = v2;
          mesh->indices[index_count++] = v3;
        }
    }

  return mesh;
}

#undef READ_LINE
