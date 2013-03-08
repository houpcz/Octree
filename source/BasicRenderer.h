// ===================================================================
// $Id: BasicRenderer.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __BASIC_RENDERER_H
#define __BASIC_RENDERER_H

#include "RendererWidget.h"
#include <vector>
using namespace std;

// Forward declarations
class Scene;
class Triangle;
class AxisAlignedBox3;
struct BvhLeaf;

class BasicRenderer :
  public RendererWidget
{
  Q_OBJECT
public:
  BasicRenderer(Scene *s);

  // This renders the whole scene
  void RenderScene();

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
};


#endif // __BASIC_RENDERER_H

