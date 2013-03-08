// ===================================================================
// $Id: AxisAlignedBox3.h,v 1.2 2007/03/02 12:41:24 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef _AxisAlignedBox3_H__
#define _AxisAlignedBox3_H__

#include "Vector3.h"
class Ray;

class AxisAlignedBox3
{
public:
  Vector3 min, max;
  // Constructors.
  AxisAlignedBox3() { }
  
  AxisAlignedBox3(const Vector3 &nMin, const Vector3 &nMax)
  {
    min = nMin; max = nMax;
  }

  void Reset() {
    min = Vector3(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT);
    max = Vector3(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);
  }

  void Include(const Vector3 &v) {
    min.Minimize(v);
    max.Maximize(v);
  }

  void Include(const AxisAlignedBox3 &b) {
    min.Minimize(b.min);
    max.Maximize(b.max);
  }
  
  Vector3 Center() const {
    return 0.5f * (min + max); 
  }

  float Center(const int axis) const {
    return 0.5f * (min[axis] + max[axis]); 
  }

  Vector3 Diagonal() const 
  { 
    return (max - min); 
  }

  float
  AxisAlignedBox3::SurfaceArea() const
  {
	Vector3 ext = max - min;
	
	return 2.0f * (ext.x * ext.y +
				   ext.x * ext.z +
				   ext.y * ext.z);
  }

  float
  AxisAlignedBox3::Volume() const
  {
	Vector3 ext = max - min;
	return ext.x * ext.y * ext.z;
  }

  Vector3 GetRandomPoint() const {
    Vector3 size = Diagonal();
    return min + Vector3(RandomValue(0.0f, size.x),
			 RandomValue(0.0f, size.y),
			 RandomValue(0.0f, size.z)); 
  }


  bool
  ComputeMinMaxT(const Ray &ray,
		 float &tmin,
		 float &tmax) const;
  
	
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
  Vector3 min = x.min;
  Vector3 max = x.max;
  min.Minimize(y.min);
  max.Maximize(y.max);
  return AxisAlignedBox3(min, max);
}

#endif //  _AxisAlignedBox3_H__
