// ===================================================================
// $Id: AppEnvironment.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

#include "AppEnvironment.h"

// Here register all the options that can be used in the environment file
void
AppEnvironment::RegisterOptions()
{
  RegisterOption("Limits.threshold", optFloat, NULL, "0.01");
  RegisterOption("Limits.small", optFloat, NULL, "1e-6");
  RegisterOption("Limits.infinity", optFloat, NULL, "1e6");
  
  RegisterOption("Scene.filename",
		 optString,
		 "scene_filename=",
		 "test.obj");
  
  RegisterOption("Bvh.maxTriangles",
		 optInt,
		 "bvh_max_triangles=",
		 "1000");

  RegisterOption("Bvh.splitType",
		 optString,
		 "bvh_split_type=",
		 "spatial_median");
  

  RegisterOption("Gvs.maxRays",
		 optInt,
		 "gvs_max_rays=",
		 "1000");

}

// End of file AppEnvironment.cpp
