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

/*
 * Apply brightness to a base color (not the light version).
 */
conge_color
apply_brightness (conge_color base, double brightness)
{
  static conge_color shades[8][4] = {
    {CONGE_GRAY, CONGE_GRAY, CONGE_WHITE, CONGE_WHITE},
    {CONGE_GRAY, CONGE_BLUE, CONGE_LBLUE, CONGE_WHITE},
    {CONGE_GRAY, CONGE_GREEN, CONGE_LGREEN, CONGE_WHITE},
    {CONGE_GRAY, CONGE_AQUA, CONGE_LAQUA, CONGE_WHITE},
    {CONGE_GRAY, CONGE_RED, CONGE_LRED, CONGE_WHITE},
    {CONGE_GRAY, CONGE_PURPLE, CONGE_LPURPLE, CONGE_WHITE},
    {CONGE_GRAY, CONGE_YELLOW, CONGE_LYELLOW, CONGE_WHITE},
    {CONGE_GRAY, CONGE_WHITE, CONGE_BWHITE, CONGE_BWHITE},
  };

  int index = floor (CLAMP (brightness, 0.0, 0.99) * 4.0);

  return shades[base][index];
}

#define SHADE_COUNT 5

/*
 * Return the properly shaded pixel based on the surface normal to be rendered
 * and the direction of lighting.
 */
conge_pixel
compute_lighting (vertex normal)
{
  static vertex light_dir = {0.0, 0.0, 1.0};
  static conge_color light_color = CONGE_BLUE;

  static char shade[SHADE_COUNT] = {'&', '%', '$', '#', '@'};

  double ambient = 0.2;
  double diffuse = dot (normal, norm (light_dir));
  int shade_index = floor (fabs (normal.z * SHADE_COUNT));

  conge_pixel pixel;

  pixel.character = shade[shade_index];
  pixel.fg = apply_brightness (light_color, MAX (ambient + diffuse, 0.0));
  pixel.bg = CONGE_BLACK;

  return pixel;
}

#undef SHADE_COUNT

void
draw_mesh_instance (conge_ctx* ctx, mesh_instance instance)
{
  int i = 0;

  vertex min, max;

  if (instance.mesh == NULL)
    return;

  /* Transform the AABB to match up with the object's orientation. */
  min = view (apply_model (instance, instance.mesh->aabb[0]));
  max = view (apply_model (instance, instance.mesh->aabb[1]));

  if (cull_aabb (min, max))
    return;

  /* Draw the visible faces. */
  for (i = 0; i < instance.mesh->index_count; i += 3)
    {
      /* Make a triangle. */
      vertex a = instance.mesh->vertices[instance.mesh->indices[i + 0]];
      vertex b = instance.mesh->vertices[instance.mesh->indices[i + 1]];
      vertex c = instance.mesh->vertices[instance.mesh->indices[i + 2]];

      /* Lighting is computed in world coordinates. */
      vertex m_a = apply_model (instance, a);
      vertex m_b = apply_model (instance, b);
      vertex m_c = apply_model (instance, c);

      vertex mvp_a = project (view (m_a));
      vertex mvp_b = project (view (m_b));
      vertex mvp_c = project (view (m_c));

      vertex mvp_normal = tri_normal (mvp_a, mvp_b, mvp_c);

      /* If the triangle points to the camera, draw it. */
      if (mvp_normal.z > 0.0)
        {
          conge_pixel fill = compute_lighting (tri_normal (m_a, m_b, m_c));

          mvp_a = norm_to_screen (mvp_a);
          mvp_b = norm_to_screen (mvp_b);
          mvp_c = norm_to_screen (mvp_c);

          conge_draw_triangle (ctx, mvp_a.x, mvp_a.y, mvp_b.x,
                               mvp_b.y, mvp_c.x, mvp_c.y, fill);
        }
    }
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

  mesh->aabb[0].x = 0.0;
  mesh->aabb[0].y = 0.0;
  mesh->aabb[0].z = 0.0;
  mesh->aabb[1] = mesh->aabb[0];

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

          /* Calculate the AABB for the mesh. */

          mesh->aabb[0].x = MIN (mesh->aabb[0].x, v->x);
          mesh->aabb[0].y = MIN (mesh->aabb[0].y, v->y);
          mesh->aabb[0].z = MIN (mesh->aabb[0].z, v->z);

          mesh->aabb[1].x = MAX (mesh->aabb[1].x, v->x);
          mesh->aabb[1].y = MAX (mesh->aabb[1].y, v->y);
          mesh->aabb[1].z = MAX (mesh->aabb[1].z, v->z);
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
