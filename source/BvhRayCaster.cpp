#include "BvhRayCaster.h"
#include "Bvh.h"
#include "Ray.h"


// cast ray
int
BvhRayCaster::CastRay(Ray &ray)
{
  return mBvh->CastRay(ray);
}

  // cast ray bucket
int
BvhRayCaster::CastRays(RayContainer &rays)
{
  int hits = 0;

  for (int i=0; i < rays.size(); i++)
	if (mBvh->CastRay(rays[i]))
	  hits++;
  
  return hits;
}
