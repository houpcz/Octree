// ===================================================================
// $Id: GvsPreprocessor.cpp,v 1.2 2007/02/28 16:45:42 bittner Exp $

// C++ standard headers

// Local headers
#include "Scene.h"
#include "Bvh.h"
#include "GvsPreprocessor.h"
#include "Environment.h"
#include "Ray.h"
#include "BvhRayCaster.h"

#ifdef USE_MLRT
#include "IntelRayCaster.h"
#endif

GvsPreprocessor::GvsPreprocessor(Scene *_scene):
  scene(_scene)
{
  Environment::GetSingleton()->GetIntValue("Gvs.maxRays",
					   envMaxRays); 
}

void
GvsPreprocessor::ComputeVisibility()
{
  rays.resize(envMaxRays);

  RayCaster *rayCaster = NULL;
  
#ifdef USE_MLRT
  if (0)
	rayCaster = new IntelRayCaster(*scene);
  else
	rayCaster = new BvhRayCaster(*scene);
#else
  rayCaster = new BvhRayCaster(*scene);
#endif
  
  long t1 = GetTime();
  cout << "Info: Casting " << envMaxRays << " random rays" << endl;
  
  // Generate random rays
  int hits = 0;
  for (int i=0; i < envMaxRays; i++) {
    // random ray origin in the box
    rays[i].origin = scene->box.GetRandomPoint();
    // second random point to which the ray is pointing to
    rays[i].direction =
      Normalize(scene->box.GetRandomPoint() - rays[i].origin);
    // the Precompute has to be called after origin/direction setup!
	//    rays[i].Precompute();
  }

  hits = rayCaster->CastRays(rays);

  long t2 = GetTime();
  float time = 1e-3*TimeDiff(t1, t2);
  cout << "Info: done in " <<  time << " seconds"<<endl;
  cout << "Info: speed =  " << envMaxRays*1e-6/time << " MRays/sec"<<endl;

  cout << "Info: #hits="<<hits<<endl;

  return;
}

// End of file GvsPreprocessor.cpp
