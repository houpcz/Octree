// ===================================================================
// $Id: GvsPreprocessor.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __GVS_PREPROCESSOR_H
#define __GVS_PREPROCESSOR_H

#include "Ray.h"

// This computes the visibility results for set of rays

class GvsPreprocessor
{
  Scene *scene;
  int envMaxRays;
  
public:
  // set of rays
  RayContainer rays;

  // preprocess a scene for ray shooting
  GvsPreprocessor(Scene *_scene);

  // computes the results of ray shooting for the specified set of rays 
  void ComputeVisibility();
};

#endif // __GVS_PREPROCESSOR_H


