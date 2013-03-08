// ===================================================================
// $Id: Ray.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __RAY_H
#define __RAY_H

#include "Vector3.h"
#include <vector>
using namespace std;

// Forward declarations
class Triangle;

// Class representing a ray: oriented halfline
struct Ray
{
public:
  // origin of the ray
  Vector3 origin;

  // normalized direction
  Vector3 direction;

  // computed intersection point along the ray
  float t;

  // intersected triangle
  Triangle *triangle;
  
  Ray(): triangle(NULL) { }
  
  void Reset() {
    triangle = NULL;
    t = MAX_FLOAT;
  }
  
  Vector3 ComputeInvertedDir() const;  
};

class RayContainer : public vector<Ray>
{
public:
  RayContainer() {}
  
};

#endif // __RAY_H

