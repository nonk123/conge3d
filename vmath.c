#include <math.h>

#include "vmath.h"

vertex
add (vertex a, vertex b)
{
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;

  return a;
}

vertex
sub (vertex a, vertex b)
{
  a.x -= b.x;
  a.y -= b.y;
  a.z -= b.z;

  return a;
}

vertex
mult (vertex v, double s)
{
  v.x *= s;
  v.y *= s;
  v.z *= s;

  return v;
}

double
length (vertex v)
{
  return sqrt (pow (v.x, 2.0) + pow (v.y, 2.0) + pow (v.z, 2.0));
}

double
dist (vertex a, vertex b)
{
  return length (sub (b, a));
}

vertex
norm (vertex v)
{
  return mult (v, 1.0 / length (v));
}

vertex
cross (vertex a, vertex b)
{
  vertex c;

  c.x = a.y * b.z - a.z * b.y;
  c.y = a.z * b.x - a.x * b.z;
  c.z = a.x * b.y - a.y * b.x;

  return c;
}

double
dot (vertex a, vertex b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

vertex
rotate_x (vertex v, double theta)
{
  vertex r = {v.x, 0.0, 0.0};
  r.y = v.y * cos (theta) - v.z * sin (theta);
  r.z = v.y * sin (theta) + v.z * cos (theta);
  return r;
}

vertex
rotate_y (vertex v, double theta)
{
  vertex r = {0.0, v.y, 0.0};
  r.x = v.x * cos (theta) + v.z * sin (theta);
  r.z = v.z * cos (theta) - v.x * sin (theta);
  return r;
}

vertex
rotate_z (vertex v, double theta)
{
  vertex r = {0.0, 0.0, v.z};
  r.x = v.x * cos (theta) - v.y * sin (theta);
  r.y = v.x * sin (theta) + v.y * cos (theta);
  return r;
}
