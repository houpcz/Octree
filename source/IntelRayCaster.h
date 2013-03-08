#ifndef __INTELCASTER_H
#define __INTELCASTER_H

#include "RayCaster.h"

class Scene;
class Triangle;

class IntelRayCaster : public RayCaster {
public:
  Triangle **triangles;
  Scene &scene;
  
  IntelRayCaster(Scene &scene);
  
  
  // cast ray
  int CastRay(Ray &ray);
  
  // cast ray bucket
  int CastRays(RayContainer &rays);

  // cast eye ray bucket
  int CastEyeRays(RayContainer &rays);

  int GetType() const {return INTEL_RAY_CASTER; }

  Triangle *
  GetTriangle(const int id);

};


#endif
