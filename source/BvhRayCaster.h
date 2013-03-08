#ifndef __BVHCASTER_H
#define __BVHCASTER_H

#include "common.h"
#include "RayCaster.h"
#include "Scene.h"

class Bvh;
class Scene;

class BvhRayCaster : public RayCaster {

public:  
  Bvh *mBvh;
  
  BvhRayCaster() {
	mBvh = NULL;
  }
  
  BvhRayCaster(Scene &scene) {
	mBvh = scene.bvh;
  }

  // cast ray
  int CastRay(mmRay &ray);
  
  // cast ray bucket
  int CastRays(RayContainer &rays);

  int CastEyeRays(RayContainer &rays) {
	return CastRays(rays);
  }

  int GetType() const {
	return BVH_RAY_CASTER;
  }

};


#endif
