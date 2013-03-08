// ===================================================================
// $Id: Ray.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __MMRAY_H
#define __MMRAY_H

#include "mmVector3.h"
#include <vector>
using namespace std;

// Forward declarations
class Triangle;

// Class representing a ray: oriented halfline
struct mmRay
{
public:
  // origin of the ray
  mmVector3 origin;

  // normalized direction
  mmVector3 direction;

  // computed intersection point along the ray
  float t;

  // intersected triangle
  Triangle *triangle;
  
  mmRay(): triangle(NULL) { }
  
  void Reset() {
    triangle = NULL;
    t = MAX_FLOAT;
  }
  
  mmVector3 ComputeInvertedDir() const;  
};

class RayContainer : public vector<mmRay>
{
public:
  RayContainer() {}
  
};

#endif // __RAY_H

