// ===================================================================
// $Id: Scene.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __SCENE_H
#define __SCENE_H

#include "Triangle.h"
#include "Bvh.h"

class Scene
{
public:
  // scene bounding box, containing all the scene primitives
  AxisAlignedBox3 box;
  // Set of scene triangles
  TriangleContainer triangles;
  // Bounding volume hierarchy 
  Bvh *bvh;
  
  Scene() {}
  void Parse(const string filename);


  void Preprocess();
  void Normalize();

  void ComputeBoundingBox();

private:
  void ParseFile(const string filename);

};

#endif //  __SCENE_H
