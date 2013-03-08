#ifndef __RAYCASTER_H
#define __RAYCASTER_H

class mmRay;
class RayContainer;

class RayCaster {
public:  

  enum {BVH_RAY_CASTER, INTEL_RAY_CASTER};

  RayCaster() {}
  virtual int CastRay(mmRay &ray) = 0;
  
  // cast ray
  virtual int CastRays(RayContainer &rays) = 0;

  virtual int CastEyeRays(RayContainer &rays) = 0;
  
  virtual int GetType() const = 0;
  
};


#endif
