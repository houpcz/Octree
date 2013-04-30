#include <stack>
#include <queue>
#include "Scene.h"
#include "RayRenderer.h"
#include "sglcontext.h"
#include "sgl.h"
#include <QSlider>
#include <QTimer>

/// like gluLookAt
void sgluLookAt(float eyex   , float eyey   , float eyez,
                float centerx, float centery, float centerz,
                float upx    , float upy    , float upz)
{
  float    sqrmag;

  /* Make rotation matrix */

  /* Z vector */
  mmVector3  z(eyex-centerx,eyey-centery,eyez-centerz);
  sqrmag = SqrMagnitude(z);
  if(sqrmag)
    z /= sqrtf(sqrmag);

  /* Y vector */
  mmVector3 y(upx,upy,upz);

  /* X vector = Y cross Z */
  mmVector3 x(CrossProd(y,z));

  sqrmag = SqrMagnitude(x);
  if(sqrmag)
    x /= sqrtf(sqrmag);

  /* Recompute Y = Z cross X */
  y = CrossProd(z,x);

  sqrmag = SqrMagnitude(y);
  if(sqrmag)
    y /= sqrtf(sqrmag);

  float m[] = {
    x.x, y.x, z.x, 0, // col 1
    x.y, y.y, z.y, 0, // col 2
    x.z, y.z, z.z, 0, // col 3
    - eyex*x.x - eyey*x.y - eyez*x.z , // col 4
    - eyex*y.x - eyey*y.y - eyez*y.z , // col 4
    - eyex*z.x - eyey*z.y - eyez*z.z , // col 4
    1.0};                              // col 4

	sglMultMatrix(m);
}

/// like gluPerspective
void sgluPerspective( float fovy, float aspect, float zNear, float zFar )
{
  fovy *= (3.1415926535/180);
  float h2 = tan(fovy/2)*zNear;
  float w2 = h2*aspect;
  sglFrustum(-w2,w2,-h2,h2,zNear,zFar);
}


// Render triangle
// Assumes that the method is called within
// GL_TRIANGLES object!

// Constructor
RayRenderer::RayRenderer(Scene *s)
  : scene(s)
{
  contextWidth = 600;
  contextHeight = 600;
  contextID = sglCreateContext(contextWidth, contextHeight);
  sglSetContext(contextID);

  trianglesPerCall = 1000;
  QSlider *slider = new QSlider(Qt::Horizontal, NULL);
  //  vl->addWidget(slider);
  //slider->show();
  slider->setRange(1, 1000);
  slider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  slider->setValue(1);
  
  connect(slider, SIGNAL(valueChanged(int)), SLOT(SetTrianglesPerCall(int)));

  // projection transformation
  sglMatrixMode(SGL_PROJECTION);
  sglLoadIdentity();
  // modelview transformation
  sglMatrixMode(SGL_MODELVIEW);
  sglLoadIdentity();

  /// BEGIN SCENE DEFINITION
  sglBeginScene();
  
  float intensity = 0.7;

  float centerY = scene->box.Center().y;
  float centerX = scene->box.Center().x;
  float centerZ = scene->box.Center().z;
  sglPointLight(centerX, scene->box.max.y + 1.5f, centerZ,
				intensity, intensity, intensity);

  sglPointLight(centerX, scene->box.min.y - 1.5f, centerZ,
				intensity, intensity, intensity);

  sglPointLight(centerX, centerY, scene->box.max.z + 2.5,
				intensity, intensity, intensity);
  
  sglMaterial(0.9f,
			  0.6f,
			  0.8f,
			  1.0f,
			  0.0f,
			  0.0f,
			  0.0f,
			  1.0f);

  TriangleContainer::const_iterator ti = scene->triangles.begin();
  for(int loop1 = 0; ti != scene->triangles.end(); ++ti, loop1++)
  {
	  sglBegin(SGL_POLYGON);
	  for(int i=0; i<3; i++)
		sglVertex3f((**ti).vertices[i].x,(**ti).vertices[i].y,(**ti).vertices[i].z);
	  sglEnd();
  }
  scene->triangles.clear();

  sglMakeOctree(true);

  cout << "Octree made." << endl;
  /*
  sglEmissiveMaterial(1.0,
					  1.0,
					  1.0,
					  2.0,
					  3.2,
					  5.0);
						
	sglBegin(SGL_POLYGON);
	sglVertex3f(scene->box.min.x, scene->box.max.y + 0.5, scene->box.min.z);
	sglVertex3f(scene->box.max.x, scene->box.max.y + 0.5, scene->box.min.z);
	sglVertex3f(scene->box.max.x, scene->box.max.y + 0.5, scene->box.max.z);
	sglVertex3f(scene->box.min.x, scene->box.max.y + 0.5, scene->box.max.z);
	sglEnd();
  */

  sglEndScene();
  /// END SCENE DEFINITION


  sglAreaMode(SGL_FILL);
  sglEnable(SGL_DEPTH_TEST);
  sglClearColor(0, 0.5f, 0, 1);
  sglClear(SGL_COLOR_BUFFER_BIT|SGL_DEPTH_BUFFER_BIT);

  // set the viewport transform
  sglViewport(0, 0, contextWidth, contextHeight);
  
  // setup the camera using appopriate projection transformation
  // note that the resolution stored in the nff file is ignored
  sglMatrixMode(SGL_PROJECTION);
  sglLoadIdentity();
  sgluPerspective(70.0, 1.0, 0.1, 100.0);
  
  // modelview transformation
  sglMatrixMode(SGL_MODELVIEW);
  sglLoadIdentity();
  sgluLookAt(0.0, 0.0, 1.0,  /* eye is at (0,0,30) */
			0.0, 0.0, 0.0,      /* center is at (0,0,0) */
			0.0, 1.0, 0.);

  float scale = 1.0f/scene->box.Diagonal().Size();
  sglScale(scale, scale, scale);

  mmVector3 translation = -scene->box.Center();
  sglTranslate(translation.x, translation.y, translation.z);

  sglRayTraceScene();
}

RayRenderer::~RayRenderer()
{
	sglFinish();
}

void
RayRenderer::RenderTriangle(const Triangle &t)
{
  mmVector3 normal = CrossProd(t.vertices[2] - t.vertices[0], t.vertices[1] - t.vertices[0]);
  glNormal3f(normal.x, normal.y, normal.z);
  glVertex3f(t.vertices[0].x, t.vertices[0].y, t.vertices[0].z);
  glVertex3f(t.vertices[1].x, t.vertices[1].y, t.vertices[1].z);
  glVertex3f(t.vertices[2].x, t.vertices[2].y, t.vertices[2].z);

  return;
}

void
RayRenderer::initializeGL()
{
  // just log GL vendor and capabilities
  RendererWidget::initializeGL();

  return;
}


void
RayRenderer::RenderScene()
{
  static long lastTime = -1;
  
  long t1 = GetRealTime();

  float frameTime = TimeDiff(lastTime, t1);
  
  lastTime = t1;
  long t2 = GetTime();


  
  float *cb = sglGetColorBufferPointer();

  if(cb)
    glDrawPixels(contextWidth, contextHeight, GL_RGB, GL_FLOAT, cb);

  update();

  return;
}
 
void
RayRenderer::RenderBox(const AxisAlignedBox3 &box)
{
  glBegin(GL_LINE_LOOP);
  glVertex3d(box.min.x, box.max.y, box.min.z );
  glVertex3d(box.max.x, box.max.y, box.min.z );
  glVertex3d(box.max.x, box.min.y, box.min.z );
  glVertex3d(box.min.x, box.min.y, box.min.z );
  glEnd();

  glBegin(GL_LINE_LOOP);
  glVertex3d(box.min.x, box.min.y, box.max.z );
  glVertex3d(box.max.x, box.min.y, box.max.z );
  glVertex3d(box.max.x, box.max.y, box.max.z );
  glVertex3d(box.min.x, box.max.y, box.max.z );
  glEnd();

  glBegin(GL_LINE_LOOP);
  glVertex3d(box.max.x, box.min.y, box.min.z );
  glVertex3d(box.max.x, box.min.y, box.max.z );
  glVertex3d(box.max.x, box.max.y, box.max.z );
  glVertex3d(box.max.x, box.max.y, box.min.z );
  glEnd();

  glBegin(GL_LINE_LOOP);
  glVertex3d(box.min.x, box.min.y, box.min.z );
  glVertex3d(box.min.x, box.min.y, box.max.z );
  glVertex3d(box.min.x, box.max.y, box.max.z );
  glVertex3d(box.min.x, box.max.y, box.min.z );
  glEnd();

  glBegin(GL_LINE_LOOP);
  glVertex3d(box.min.x, box.min.y, box.min.z );
  glVertex3d(box.max.x, box.min.y, box.min.z );
  glVertex3d(box.max.x, box.min.y, box.max.z );
  glVertex3d(box.min.x, box.min.y, box.max.z );
  glEnd();

  glBegin(GL_LINE_LOOP);
  glVertex3d(box.min.x, box.max.y, box.min.z );
  glVertex3d(box.max.x, box.max.y, box.min.z );
  glVertex3d(box.max.x, box.max.y, box.max.z );
  glVertex3d(box.min.x, box.max.y, box.max.z );

  glEnd();

  return;
}

void
RayRenderer::RenderBvhLeaf(BvhLeaf *leaf)
{
  bool useGlLists = true;

  if (leaf->glList == 0) {
	if (useGlLists) {
	  leaf->glList = glGenLists(1);
	  if (leaf->glList != 0)
		glNewList(leaf->glList, GL_COMPILE);
	}
	
	Triangle *t = scene->bvh->triangles[leaf->first];

#if 1
	glColor3ub(t->color.r, t->color.g, t->color.b);
#else
	float x = 100.0f*leaf->box.SurfaceArea()/scene->box.SurfaceArea();
	mmColor color = RainbowColorMapping(x);
#if 0
	float scale = trianglesPerCall/1000000.0f;
	mmColor color = RainbowColorMapping(scale*leaf->depth);
#endif
	glColor3ub(color.r, color.g, color.b);
#endif	

	static int textureId = 0;
	glBindTexture(GL_TEXTURE_2D, textures[textureId]);
	textureId = (textureId + 1 )%textures.size();
	
	// issue render call
	glBegin(GL_TRIANGLES);
	
	for (int i=leaf->first; i <= leaf->last; i++)
	  RenderTriangle(*scene->bvh->triangles[i]);
	
	glEnd();

	if (leaf->glList != 0)
	  glEndList();
	
  }
  
  if (leaf->glList != 0)
	glCallList(leaf->glList);
}

struct BvhTraversalEntry {
  public:
  BvhNode *node;
  float priority;
  BvhTraversalEntry() {}
  BvhTraversalEntry(BvhNode *n, const float p):
	node(n), priority(p) {}
  friend bool operator<(const BvhTraversalEntry &a, const BvhTraversalEntry &b) {
	return a.priority < b.priority;
  }
};

void
RayRenderer::RenderBvhBoxes()
{
  stack<BvhNode *> nodeStack;
  
  nodeStack.push(scene->bvh->root);
  
  while (!nodeStack.empty()) {
	BvhNode *node = nodeStack.top();
	nodeStack.pop();
	if (node->IsLeaf()) {
	  RenderBox(node->box);
	}
	else {
	  BvhInterior *interior = (BvhInterior *)node;
	  // render from front to back!
	  nodeStack.push(interior->back);
	  nodeStack.push(interior->front);
	}
  }
}

void
RayRenderer::RenderBvh()
{
  priority_queue<BvhTraversalEntry> nodeStack;

  nodeStack.push(BvhTraversalEntry(scene->bvh->root, 1.0f));
  while (!nodeStack.empty()) {
	BvhNode *node = nodeStack.top().node;
	nodeStack.pop();
	if (node->IsLeaf()) {
	  // render triangles in this leaf
	  RenderBvhLeaf((BvhLeaf *)node);
	}
	else {
	  BvhInterior *interior = (BvhInterior *)node;
	  // render from front to back!
	  nodeStack.push(BvhTraversalEntry(interior->back, 1.0f));
	  nodeStack.push(BvhTraversalEntry(interior->front, 1.0f));
	}
  }
}

void
RayRenderer::SetTrianglesPerCall(int value)
{
  float exp = (value*1e-3f)*6.0f;
  trianglesPerCall = (int) pow((long double)10.0, (long double)exp);
}

// End of file BasicRenderer.cpp
