#include "common.h"
#include "IntelRayCaster.h"
#include "Environment.h"
#include "Scene.h"
#include "Triangle.h"
#include "Ray.h"
#include "ArchModeler2MLRT.hxx"


IntelRayCaster::IntelRayCaster(Scene &_scene):
  scene(_scene)
{
  string kdtree, sceneName;
  
  Environment::GetSingleton()->GetStringValue("Scene.filename", sceneName);

  kdtree = ReplaceSuffix(sceneName, ".obj", ".kdf");
  
  if (!mlrtaLoadAS(kdtree.c_str())) {
	cerr<<"Intel ray caster could not be inititalized\n";
	exit(1);
  }

#if 0
  float *pv;
  int nv;
  float *ptri;
  int ntri;
  float *pn;
  int nn;
  mlrta_pointers(pv, nv, ptri, ntri, pn, nn);
#endif

  //copy the pointers
  triangles = new Triangle*[scene.triangles.size()];
  for (int i=0; i < scene.triangles.size(); i++)
	triangles[i] = scene.triangles[i];
  
}

// cast ray
int
IntelRayCaster::CastRay(Ray &ray)
{
  float dist;
  double normal[3];
  
  int hittriangle = mlrtaIntersectAS(
									 &ray.origin.x,
									 &ray.direction.x,
									 normal,
									 dist);
  
  
  if (hittriangle != -1 ) {
	//	Intersectable *intersect = mPreprocessor.GetParentObject(hittriangle);
	ray.triangle = GetTriangle(hittriangle);
	ray.t = dist;
	return 1;
  }
  ray.triangle = NULL;
  return 0;
}

Triangle *
IntelRayCaster::GetTriangle(const int id)
{
  return triangles[id];
}

// cast ray bucket
int
IntelRayCaster::CastRays(RayContainer &rays)
{
  const int packetSize = 16;
  int hits = 0;
  int packets = rays.size()/packetSize;
  int k = 0;
  int hit_triangles[packetSize];
  float dist[packetSize];
  int i,j;

  Vector3 min = scene.box.min;
  Vector3 max = scene.box.max;
  for (i=0; i < packets; i++) {
	for (j = 0; j < packetSize; j++, k++) 
	  mlrtaStoreRayAS16(&rays[k].origin.x,
						&rays[k].direction.x,
						j);

	mlrtaTraverseGroupAS16(&min.x,
						   &max.x,
						   hit_triangles,
						   dist);
	
	k-=packetSize;
	for (j = 0; j < packetSize; j++, k++) 
	  if (hit_triangles[j] != -1) {
		rays[k].triangle = GetTriangle(hit_triangles[j]);
		rays[k].t = dist[j];
		hits++;
	  } else
		rays[k].triangle = NULL;
  }

  for (; k < rays.size(); k++)
	hits+=CastRay(rays[k]);
  
  return hits;
}



int
IntelRayCaster::CastEyeRays(RayContainer &rays)
{
  const int packetSize = 4;
  int hits = 0;
  int packets = rays.size()/packetSize;
  int k = 0;
  int hit_triangles[packetSize];
  float dist[packetSize];
  int i,j;

  Vector3 min = scene.box.min;
  Vector3 max = scene.box.max;
  for (i=0; i < packets; i++) {
	for (j = 0; j < packetSize; j++, k++) 
	  mlrtaStoreRayASEye4(&rays[k].origin.x,
						  &rays[k].direction.x,
						  j);
	
	mlrtaTraverseGroupASEye4(&min.x,
							 &max.x,
							 hit_triangles,
							 dist);
	
	k-=packetSize;
	for (j = 0; j < packetSize; j++, k++) 
	  if (hit_triangles[j] != -1) {
		rays[k].triangle = GetTriangle(hit_triangles[j]);
		rays[k].t = dist[j];
		hits++;
	  } else
		rays[k].triangle = NULL;
  }

  for (; k < rays.size(); k++)
	hits+=CastRay(rays[k]);
  
  return hits;
}
