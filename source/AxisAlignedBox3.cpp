// ===================================================================
// $Id: AxisAlignedBox3.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

#include "mmRay.h"
#include "sglStructures.h"
#include "AxisAlignedBox3.h"

#include "aabb_triangle.h"
bool AxisAlignedBox3::CollisionTriangle(sglTriangle * triangle)
{
	mmVector3 center = Center();
	mmVector3 diagonal = Diagonal();

	float boxcenter[3] = {center.x, center.y, center.z};
	float boxhalfsize[3] = {diagonal.x / 2.0f, diagonal.y / 2.0f, diagonal.z / 2.0f};
	float triverts[3][3] = { {triangle->point1.x, triangle->point1.y, triangle->point1.z},
						   {triangle->point1.x, triangle->point1.y, triangle->point1.z},
						   {triangle->point1.x, triangle->point1.y, triangle->point1.z}};

	return triBoxOverlap(boxcenter, boxhalfsize, triverts);
}

// Compute the intersection of a ray with the box
bool
AxisAlignedBox3::ComputeMinMaxT(const mmRay &ray,
								float &tmin,
								float &tmax) const
{
  const float dirEps = 1e-8f;
  register float minx, maxx;
  
  if (fabs(ray.direction.x) < dirEps) {
    if (min.x < ray.origin.x && max.x > ray.origin.x) {
      minx = -MAXFLOAT;
      maxx = MAXFLOAT;
    }
    else
      return false;
  }
  else {
    float t1 = (min.x - ray.origin.x) / ray.direction.x;
    float t2 = (max.x - ray.origin.x) / ray.direction.x;
    if (t1 < t2) {
      minx = t1;
      maxx = t2;
    }
    else {
      minx = t2;
      maxx = t1;
    }
    if (maxx < 0.0)
      return false;
  }

  tmin = minx;
  tmax = maxx;
  
  if (fabs(ray.direction.y) < dirEps) {
    if (min.y < ray.origin.y && max.y > ray.origin.y) {
      minx = -MAXFLOAT;
      maxx = MAXFLOAT;
    }
    else
      return false;
  }
  else {
    float t1 = (min.y - ray.origin.y) / ray.direction.y;
    float t2 = (max.y - ray.origin.y) / ray.direction.y;
    if (t1 < t2) {
      minx = t1;
      maxx = t2;
    }
    else {
      minx = t2;
      maxx = t1;
    }
    if (maxx < 0.0)
      return false;
  }

  if (minx > tmin)
    tmin = minx;
  if (maxx < tmax)
    tmax = maxx;
  
  if (fabs(ray.direction.z) < dirEps) {
    if (min.z < ray.origin.z && max.z > ray.origin.z) {
      minx = -MAXFLOAT;
      maxx = MAXFLOAT;
    }
    else
      return false;
  }
  else {
    float t1 = (min.z - ray.origin.z) / ray.direction.z;
    float t2 = (max.z - ray.origin.z) / ray.direction.z;
    if (t1 < t2) {
      minx = t1;
      maxx = t2;
    }
    else {
      minx = t2;
      maxx = t1;
    }
    if (maxx < 0.0)
      return false;
  }

  if (minx > tmin)
    tmin = minx;

  if (maxx < tmax)
    tmax = maxx;
  
  return tmin <= tmax; // yes, intersection was found
}

// Compute the intersection of a ray with the box
bool
AxisAlignedBox3::ComputeMinMaxT(const Ray &ray,
								float &tmin,
								float &tmax) const
{
  const float dirEps = 1e-8f;
  register float minx, maxx;
  
  if (fabs(ray.direction.x) < dirEps) {
    if (min.x < ray.origin.x && max.x > ray.origin.x) {
      minx = -MAXFLOAT;
      maxx = MAXFLOAT;
    }
    else
      return false;
  }
  else {
    float t1 = (min.x - ray.origin.x) / ray.direction.x;
    float t2 = (max.x - ray.origin.x) / ray.direction.x;
    if (t1 < t2) {
      minx = t1;
      maxx = t2;
    }
    else {
      minx = t2;
      maxx = t1;
    }
    if (maxx < 0.0)
      return false;
  }

  tmin = minx;
  tmax = maxx;
  
  if (fabs(ray.direction.y) < dirEps) {
    if (min.y < ray.origin.y && max.y > ray.origin.y) {
      minx = -MAXFLOAT;
      maxx = MAXFLOAT;
    }
    else
      return false;
  }
  else {
    float t1 = (min.y - ray.origin.y) / ray.direction.y;
    float t2 = (max.y - ray.origin.y) / ray.direction.y;
    if (t1 < t2) {
      minx = t1;
      maxx = t2;
    }
    else {
      minx = t2;
      maxx = t1;
    }
    if (maxx < 0.0)
      return false;
  }

  if (minx > tmin)
    tmin = minx;
  if (maxx < tmax)
    tmax = maxx;
  
  if (fabs(ray.direction.z) < dirEps) {
    if (min.z < ray.origin.z && max.z > ray.origin.z) {
      minx = -MAXFLOAT;
      maxx = MAXFLOAT;
    }
    else
      return false;
  }
  else {
    float t1 = (min.z - ray.origin.z) / ray.direction.z;
    float t2 = (max.z - ray.origin.z) / ray.direction.z;
    if (t1 < t2) {
      minx = t1;
      maxx = t2;
    }
    else {
      minx = t2;
      maxx = t1;
    }
    if (maxx < 0.0)
      return false;
  }

  if (minx > tmin)
    tmin = minx;

  if (maxx < tmax)
    tmax = maxx;
  
  return tmin <= tmax; // yes, intersection was found
}


// End of file AxisAlignedBox3.cpp
