// ===================================================================
// $Id: Triangle.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

// local headers
#include "Triangle.h"
#include "Ray.h"

// Computes a ray-triangle intersection
void
Triangle::CastRay(Ray &ray)
{	
  //////////////
  // specialised triangle ray casting version
  
  ///////////
  //-- calc ray-plane intersection
  
  // get triangle edge vectors and plane normal
  const Vector3 u = vertices[0] - vertices[1];
  const Vector3 v = vertices[2] - vertices[1];
  
  // cross product
  //  Vector3 normal = Normalize(CrossProd(v, u));
  Vector3 normal = CrossProd(v, u);
  
  // ray direction vector
  const Vector3 dir = ray.direction;
  const Vector3 w0 = ray.origin - vertices[1];
  
  // params to calc ray-plane intersect
  const float a = -DotProd(normal, w0);
  const float b = DotProd(normal, dir);
  
  // check for division by zero
  if (fabs(b) < Limits::Small) {   
    // ray is parallel to triangle plane
    if (a == 0) {
      // ray lies in triangle plane
      return;
    }
    else {
      // ray disjoint from plane
      return;
    }
  }

  // distance from origin of ray to plane
  float t = a / b;
  
  if (t < 0.0) // ray goes away from triangle
  {
    return ; // => no intersect
  }
  // already found nearer intersection
  else {
    if (t >= ray.t) {
      return ;
    }
  }
  
  /////////////////
  //-- found intersection point
  //-- check if it is inside triangle
  //-- the method uses Barycentric coordinates
  
  const Vector3 pt = ray.origin + t * dir;
  const Vector3 w = pt - vertices[1];
  
  const float uu = DotProd(u, u);
  const float uv = DotProd(u, v);
  const float vv = DotProd(v, v);
  
  const float wu = DotProd(w, u);
  const float wv = DotProd(w, v);
  const float D = uv * uv - uu * vv;
  
  // get and test parametric coords
  const float s = (uv * wv - vv * wu) / D;
  
  if ((s < 0.0f) || (s > 1.0f)) 
    return; // pt is outside triangle
  
  const float s2 = (uv * wu - uu * wv) / D;
  
  if ((s2 < 0.0f) || ((s + s2) > 1.0f))
    return;  // pt is outside triangle

  // Ray intersects the triangle
  ray.t = t;
  ray.triangle = this;
}

// End of file Triangle.cpp
