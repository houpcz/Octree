// ===================================================================
// $Id: BasicRenderer.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __RAY_RENDERER_H
#define __RAY_RENDERER_H

#include "RendererWidget.h"
#include "sglcontext.h"
#include <vector>
using namespace std;

// Forward declarations
class Scene;
class Triangle;
class AxisAlignedBox3;
struct BvhLeaf;

class RayRenderer :  
  public RendererWidget
{ 
  Q_OBJECT
public:
  RayRenderer(Scene *s);

  // This renders the whole scene
  void RenderScene();
  ~RayRenderer();

  void
  RenderTriangle(const Triangle &t);

  void
  RenderBox(const AxisAlignedBox3 &box);

  void
  RenderBvh();

  void
  RenderBvhBoxes();

  void
  RenderBvhLeaf(BvhLeaf *leaf);

public slots:

void SetTrianglesPerCall(int value);
  
protected:

  // this method is called during GL initialization stage
  void initializeGL();

  Scene *scene;
  int trianglesPerCall;
  vector<unsigned int> textures;
  int contextWidth;
  int contextHeight;
  int contextID;
};


#endif // __BASIC_RENDERER_H

