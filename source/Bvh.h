// ===================================================================
// $Id: Bvh.h,v 1.2 2007/03/02 12:41:24 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __BVH_H
#define __BVH_H

#include "Triangle.h"
#include "AxisAlignedBox3.h"

// Forward declarations
class Scene;

struct BvhNode
{
  char axis;
  unsigned char depth;
  short flags;
  AxisAlignedBox3 box;
  
  BvhNode():axis(-1) {}
  bool IsLeaf() { return axis == -1; }
};


struct BvhInterior :
  public BvhNode
{
  BvhInterior():back(NULL), front(NULL) { }
  BvhNode *back;
  BvhNode *front;
};


struct BvhLeaf :
  public BvhNode
{
  mmColor color;
  int texture;
  int glList;
  
  BvhLeaf(): first(-1), glList(0), texture(0) { }
  
  bool Empty() const { return first == -1; }
  int Count() const { return last - first + 1; }
  // indices to first and last triangle in the triangle array
  // assumes the traingle are placed in continuous chunk of memory
  // however this need not be a global array!
  int first;
  // one after the last triangle!
  int last;
};

class Bvh
{
public:
  struct RayTraversalInfo
  {
    float t;
    BvhNode *node;
    
    RayTraversalInfo() {}
    RayTraversalInfo(const float _t, BvhNode *n):
      t(_t), node(n) {}
    
    friend bool operator<(const RayTraversalInfo &a,
			  const RayTraversalInfo &b)
    {
      return a.t > b.t;
    }
  };

  enum {SPATIAL_MEDIAN, OBJECT_MEDIAN, SAH};
  // the root of the hieararchy
  BvhNode *root;
  // pinters to the scene traingles
  vector<Triangle *> triangles;
  int envMaxTriangles;
  int envSplitType;
  int numberOfNodes;

  Bvh(Scene &scene);

  // construct the BVH using options specified in the AppEnvironment
  void Construct();

  BvhNode *
  SubdivideLeaf(BvhLeaf *leaf, const int axis);

  // The method of subdividing objects into halves in some axis
  // using spatial median
  int
  SortTrianglesSpatialMedian(BvhLeaf *leaf, const int axis);

  // The method of subdividing objects into halves in some axis
  // using object median
  int
  SortTrianglesObjectMedian(BvhLeaf *leaf, const int axis);

  // sort triangles along the axis with respect to the splitting plane
  // given by axis/position
  // return index of the first tiangles belong to the front of the splitting
  // plane
  
  int
  SortTriangles(BvhLeaf *leaf, const int axis, const float position);

  // select splitting plane using SAH - returns the position of the
  // splitting plane
  float
  SelectPlaneSah(BvhLeaf *leaf, int &axis, float &minCost);

  // evaluate the SAH cost of a particulkar splitting plane
  float
  EvaluateSahCost(BvhLeaf *leaf, const int axis, const float position);


  void
  SumSurfaceAreas(float &interiorSA,
				  float &leafSA,
				  float &interiorVol,
				  float &leafVol
				  );

  // Update children nodes recursively
  void
  UpdateBoxes(BvhNode *node);

  // Update box for a leaf node
  void
  UpdateLeafBox(BvhLeaf *leaf);

  // Cast a ray (ray shooting) using BVH
  bool
  CastRay(mmRay &ray);
};

#endif //  __BVH_H

