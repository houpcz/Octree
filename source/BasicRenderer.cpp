#include <stack>
#include <queue>
#include "Scene.h"
#include "BasicRenderer.h"
#include <QSlider>
#include <QTimer>

// Render triangle
// Assumes that the method is called within
// GL_TRIANGLES object!

// Constructor
BasicRenderer::BasicRenderer(Scene *s)
  : scene(s)
{
  
  trianglesPerCall = 1000;
  QSlider *slider = new QSlider(Qt::Horizontal, NULL);
  //  vl->addWidget(slider);
  //slider->show();
  slider->setRange(1, 1000);
  slider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  slider->setValue(1);
  
  connect(slider, SIGNAL(valueChanged(int)), SLOT(SetTrianglesPerCall(int)));

  // QTimer *timer = new QTimer(this);
  // connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  // timer->start(1);
}

void
BasicRenderer::RenderTriangle(const Triangle &t)
{
  //  mmVector3 normal = CrossProd(t.vertices[2] - t.vertices[0], t.vertices[1] - t.vertices[0]);
  //  glNormal3f(normal.x, normal.y, normal.z);
  glVertex3f(t.vertices[0].x, t.vertices[0].y, t.vertices[0].z);
  glVertex3f(t.vertices[1].x, t.vertices[1].y, t.vertices[1].z);
  glVertex3f(t.vertices[2].x, t.vertices[2].y, t.vertices[2].z);

  return;
}

void
BasicRenderer::initializeGL()
{
  // just log GL vendor and capabilities
  RendererWidget::initializeGL();
  
  float intensity = 0.03;

  float centerY = scene->box.Center().y; 
  float centerX = scene->box.Center().x;
  float centerZ = scene->box.Center().z;

  GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light_diffuse[] =  { intensity * 0.8, intensity * 0.6, intensity * 0.8, 1.0 };
  GLfloat light_specular[] = { intensity * 0.8, intensity * 0.6, intensity * 0.8, 1.0 };
  GLfloat light_position1[] = { centerX, scene->box.max.y + 1.5f, centerZ, 0.0 };
  GLfloat light_position2[] = { centerX, scene->box.max.y - 1.5f, centerZ, 0.0 };
  GLfloat light_position3[] = { centerX, centerY, scene->box.max.z + 2.5, 0.0 };

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position1);

  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position2);

  glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT2, GL_POSITION, light_position3);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);
  glShadeModel(GL_FLAT);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);


  return;
}


void
BasicRenderer::RenderScene()
{
  float scale = 1.0f/scene->box.Diagonal().Size();
  glScalef(scale, scale, scale);

  mmVector3 translation = -scene->box.Center();
  glTranslatef(translation.x, translation.y, translation.z);
  
  static long lastTime = -1;
  long t1 = GetRealTime();
  float frameTime = TimeDiff(lastTime, t1);
  lastTime = t1;
  openRasterizeScene();
  long t2 = GetTime();

  // Here is the visualization of BVH cells
  //  bool renderBvhFlag = true;
  //  if (renderBvhFlag)
  //    RenderBvh();
  
  char buff[256];
  sprintf(buff, "FPS:%05.1f  time:%05.0f  #triangles:%d #trianglesPercall:%d",
	  1e3f/frameTime,
	  frameTime,
	  scene->triangles.size(),
	  trianglesPerCall);
  
  glColor3f(1.0f, 1.0f, 1.0f);
  renderText(10,20,buff);
  //  cout<<buff<<endl;
  
  update();

  return;
}
 
void
BasicRenderer::RenderBox(const AxisAlignedBox3 &box)
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
BasicRenderer::RenderBvhLeaf(BvhLeaf *leaf)
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
BasicRenderer::RenderBvhBoxes()
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
BasicRenderer::RenderBvh()
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
BasicRenderer::SetTrianglesPerCall(int value)
{
  float exp = (value*1e-3f)*6.0f;
  trianglesPerCall = (int) pow((long double)10.0, (long double)exp);
}

// End of file BasicRenderer.cpp
