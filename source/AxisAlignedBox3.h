// ===================================================================
// $Id: AxisAlignedBox3.h,v 1.2 2007/03/02 12:41:24 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef _AxisAlignedBox3_H__
#define _AxisAlignedBox3_H__

#include "mmVector3.h"
#include "sglprimitive.h"
class mmRay;
class Ray;

class AxisAlignedBox3
{
public:
  mmVector3 min, max;
  // Constructors.
  AxisAlignedBox3() { }
  
  AxisAlignedBox3(const mmVector3 &nMin, const mmVector3 &nMax)
  {
    min = nMin; max = nMax;
  }

  AxisAlignedBox3 GetOctant(int octantID)
  {
	  mmVector3 center = Center();

	  switch(octantID)
	  {
		case 0 :
			return AxisAlignedBox3(mmVector3(min.x, min.y, min.z), mmVector3(center.x, center.y, center.z));
		case 1 :
			return AxisAlignedBox3(mmVector3(center.x, min.y, min.z), mmVector3(max.x, center.y, center.z));
		case 2 :
			return AxisAlignedBox3(mmVector3(min.x, center.y, min.z), mmVector3(center.x, max.y, center.z));
		case 3 :
			return AxisAlignedBox3(mmVector3(center.x, center.y, min.z), mmVector3(max.x, max.y, center.z));
		case 4 :
			return AxisAlignedBox3(mmVector3(min.x, min.y, center.z), mmVector3(center.x, center.y, max.z));
		case 5 :
			return AxisAlignedBox3(mmVector3(center.x, min.y, center.z), mmVector3(max.x, center.y, max.z));
		case 6 :
			return AxisAlignedBox3(mmVector3(min.x, center.y, center.z), mmVector3(center.x, max.y, max.z));
		case 7 :
			return AxisAlignedBox3(mmVector3(center.x, center.y, center.z), mmVector3(max.x, max.y, max.z));
	  }
  }

  void Reset() {
    min = mmVector3(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT);
    max = mmVector3(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);
  }

  void Include(const Vector3 &v) {
	  Include(mmVector3(v.x, v.y, v.z));
  }

  void Include(const mmVector3 &v) {
    min.Minimize(v);
    max.Maximize(v);
  }

  void Include(const AxisAlignedBox3 &b) {
    min.Minimize(b.min);
    max.Maximize(b.max);
  }
  
  mmVector3 Center() const {
    return 0.5f * (min + max); 
  }

  float Center(const int axis) const {
    return 0.5f * (min[axis] + max[axis]); 
  }

  mmVector3 Diagonal() const 
  { 
    return (max - min); 
  }

  float
  AxisAlignedBox3::SurfaceArea() const
  {
	mmVector3 ext = max - min;
	
	return 2.0f * (ext.x * ext.y +
				   ext.x * ext.z +
				   ext.y * ext.z);
  }

  float
  AxisAlignedBox3::Volume() const
  {
	mmVector3 ext = max - min;
	return ext.x * ext.y * ext.z;
  }

  mmVector3 GetRandomPoint() const {
    mmVector3 size = Diagonal();
    return min + mmVector3(RandomValue(0.0f, size.x),
			 RandomValue(0.0f, size.y),
			 RandomValue(0.0f, size.z)); 
  }


  bool
  ComputeMinMaxT(const mmRay &ray,
		 float &tmin,
		 float &tmax) const;

  bool
  ComputeMinMaxT(const Ray &ray,
		 float &tmin,
		 float &tmax) const;
  
  bool CollisionTriangle(sglTriangle * triangle);
	
  friend inline AxisAlignedBox3 Union(const AxisAlignedBox3 &x,
									  const AxisAlignedBox3 &y);
  
  friend ostream &operator<<(ostream &s, const AxisAlignedBox3 &b)
  {
    return s << b.min << ":" << b.max;
  }
  
};  

inline AxisAlignedBox3
Union(const AxisAlignedBox3 &x, const AxisAlignedBox3 &y)
{
  mmVector3 min = x.min;
  mmVector3 max = x.max;
  min.Minimize(y.min);
  max.Maximize(y.max);
  return AxisAlignedBox3(min, max);
}

#endif //  _AxisAlignedBox3_H__

