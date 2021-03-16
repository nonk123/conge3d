/*
 * vmath.h - vector- and misc. math.
 */

#ifndef VMATH_H
#define VMATH_H

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define CLAMP(V, MIN_V, MAX_V) (MIN (MAX ((V), (MIN_V)), (MAX_V)))

/* A simple 3D vector. */
typedef struct vertex vertex;
struct vertex { double x, y, z; };

/* Vertex index in the indices array. */
typedef short int v_index;

/*
 * Add two vectors together.
 */
vertex add (vertex, vertex);

/*
 * Subtract the second vector from the first.
 */
vertex sub (vertex, vertex);

/*
 * Multiply the given vector by a scalar.
 */
vertex mult (vertex, double);

/*
 * Compute the length of the given vector.
 */
double length (vertex);

/*
 * Return the distance between two vectors.
 */
double dist (vertex, vertex);

/*
 * Normalize the given vector by setting its length to 1.
 */
vertex norm (vertex);

/*
 * Find the cross product of two vectors.
 */
vertex cross (vertex, vertex);

/*
 * Calculate the dot product of two vectors.
 */
double dot (vertex, vertex);

/* The functions below rotate a vector by a given angle. */

vertex rotate_x (vertex, double);
vertex rotate_y (vertex, double);
vertex rotate_z (vertex, double);

#endif /* VMATH_H */
