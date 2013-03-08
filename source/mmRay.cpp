// ===================================================================
// $Id: Ray.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

// local headers
#include "mmRay.h"

mmVector3
mmRay::ComputeInvertedDir() const
{
  const float eps = 1e-6f;
  const float invEps = 1e6f;

  mmVector3 invDirection;
  
  // it does change the ray direction very slightly,
  // but the size direction vector is not practically changed  
  if (fabs(direction.x) < eps) {
    if (direction.x < 0.0)
      invDirection.x = -invEps;
    else
      invDirection.x = invEps;
  }
  else
    invDirection.x = 1.0f / direction.x;
  
  if (fabs(direction.y) < eps) {
    if (direction.y < 0.0)
      invDirection.y = -invEps;
    else
      invDirection.y = invEps;
  }
  else
    invDirection.y = 1.0f / direction.y;
  
  if (fabs(direction.z) < eps) {
    if (direction.z < 0.0f)
      invDirection.z = -invEps;
    else
      invDirection.z = invEps;
  }
  else
    invDirection.z = 1.0f / direction.z;

  return invDirection;
}

// End of file Ray.cpp
