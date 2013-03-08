// ===================================================================
// $Id: Triangle.h,v 1.2 2007/03/02 12:41:24 bittner Exp $
// This file has been written by Jiri Bittner, October 2006


#ifndef __TRIANGLE_H
#define __TRIANGLE_H

#include <vector>
using namespace std;

#include "mmColor.h"
#include "mmVector3.h"
#include "AxisAlignedBox3.h"
#include "common.h"

// Forward declarations
class Ray;

// A triangle represented by 3 vertices and one color
// for the whole triangle
struct Triangle
{
  unsigned short boundingBoxIndices;
  mmVector3 vertices[3];
  mmColor color;
  
  Triangle() {}
  
  Triangle(const mmVector3 &a, 
		   const mmVector3 &b,
		   const mmVector3 &c) {
    vertices[0] = a;
    vertices[1] = b;
    vertices[2] = c;
	ComputeBoundingBoxIndices();
  }

  void ComputeBoundingBoxIndices() {
	int axis;
	boundingBoxIndices = 0;

	for (axis=2; axis >=0; axis--) {
	  int i = 0;
	  if (vertices[1][axis] > vertices[i][axis])
		i = 1;
	  if (vertices[2][axis] > vertices[i][axis])
		i = 2;
	  boundingBoxIndices = (boundingBoxIndices<<2) + i;
	}

	for (axis=2; axis >=0; axis--) {
	  int i = 0;
	  if (vertices[1][axis] < vertices[i][axis])
		i = 1;
	  if (vertices[2][axis] < vertices[i][axis])
		i = 2;
	  boundingBoxIndices = (boundingBoxIndices<<2) + i;
	}

	//	cout<<boundingBoxIndices<<endl;
  }
  
  mmVector3 GetMin() const {
    return mmVector3(GetMin(0), GetMin(1), GetMin(2));
  }

  mmVector3 GetMax() const {
    return mmVector3(GetMax(0), GetMax(1), GetMax(2));
  }
  
  float GetMin(const int axis) const {
	return vertices[(boundingBoxIndices>>(axis<<1))&3][axis];
	//return Min(Min(vertices[0][axis], vertices[1][axis]), vertices[2][axis]);
  }
  
  float GetMax(const int axis) const {
	return vertices[(boundingBoxIndices>>(6+(axis<<1)))&3][axis];
	//	return Max(Max(vertices[0][axis], vertices[1][axis]), vertices[2][axis]);
  }

  float GetCenter(const int axis) {
    return (vertices[0][axis] + vertices[1][axis] + vertices[2][axis])/3.0f;
  }

  AxisAlignedBox3 GetBox() {
	return AxisAlignedBox3(GetMin(), GetMax());
  }
  
  void CastRay(Ray &ray);
};

// Just a container of triangles
class TriangleContainer :
  public vector<Triangle *>
{
public:
  TriangleContainer() {}
  
};

#endif //  __TRIANGLE_H
