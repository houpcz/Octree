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
  //  Vector3 normal = CrossProd(t.vertices[2] - t.vertices[0], t.vertices[1] - t.vertices[0]);
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

  if (1) {
    GLfloat mat_ambient[] = {  0.5, 0.5, 0.5, 1.0  };
    GLfloat mat_diffuse[] = {  1.0, 1.0, 1.0, 1.0  };
	
    GLfloat light_ambient[] = {  0.2, 0.2, 0.2, 1.0  };
    GLfloat light_diffuse[] = {  0.4, 0.4, 0.4, 1.0  };
    GLfloat lmodel_ambient[] = {  0.3, 0.3, 0.3, 1.0  };
	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // set position of the light
    GLfloat infinite_light[] = {  1.0, 0.8, 1.0, 0.0  };
    glLightfv (GL_LIGHT0, GL_POSITION, infinite_light);
    
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //   glColorMaterial( GL_FRONT_AND_BACK, GL_SPECULAR);
    glEnable(GL_COLOR_MATERIAL);
  }
  
  qglClearColor(QColor(0,0,128));
  glShadeModel(GL_FLAT);
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glShadeModel( GL_FLAT );
  //glEnable(GL_AUTO_NORMAL);
  glDepthFunc( GL_LESS );
  glEnable( GL_DEPTH_TEST );

  glDisable(GL_LIGHTING);

  // glEnable( GL_NORMALIZE );


  textures.resize(2);

  // generate two dummy textures
  for (int i=0; i < textures.size(); i++) {
    glGenTextures(1, &textures[i]);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    int width = 512;
    int height = 512;
    
    unsigned char *buffer = new unsigned char[width*height*3];
    // generate a texture
    glTexImage2D(GL_TEXTURE_2D,
				 0,
				 3,
				 width,
				 height,
				 0,
				 GL_RGB,
				 GL_UNSIGNED_BYTE,
				 buffer
				 );
    delete buffer;
  }
  
  glEnable(GL_TEXTURE_2D);

  return;
}


void
BasicRenderer::RenderScene()
{
  float scale = 1.0f/scene->box.Diagonal().Size();
  glScalef(scale, scale, scale);

  Vector3 translation = -scene->box.Center();
  glTranslatef(translation.x, translation.y, translation.z);

  // Render the whole scene
  // Just loop over all triangles
  TriangleContainer::const_iterator ti = scene->triangles.begin();
  glColor3f(1.0f, 0.0f, 0.0f);
  
  static long lastTime = -1;
  
  long t1 = GetRealTime();

  float frameTime = TimeDiff(lastTime, t1);
  
  lastTime = t1;

  int i;
  bool wireframe = mWireframe;
  int calls = 0;

  bool useBvh = true;
  
  if (!wireframe) {
	
	if (useBvh) {
	  RenderBvh();
	  if (1)
		RenderBvhBoxes();

	} else {
	  
	  while (ti != scene->triangles.end()) {
		calls++;

		// change gl state!  
#if 0
		glColor3ub((*ti)->color.r, (*ti)->color.g, (*ti)->color.b);
#else
		int maxCalls = scene->triangles.size()/trianglesPerCall;
		mmColor color = RainbowColorMapping(calls/(float)maxCalls);
		glColor3ub(color.r, color.g, color.b);
#endif		
		static int textureId = 0;
		glBindTexture(GL_TEXTURE_2D, textures[textureId]);
		textureId = (textureId + 1 )%textures.size();
		
		// issue render call
		glBegin(GL_TRIANGLES);
		for (i=0; ti != scene->triangles.end() && i < trianglesPerCall; ti++, i++)
		  RenderTriangle(**ti);
		glEnd();  
	  }
	}
  }
  else {
    while (ti != scene->triangles.end()) {
      for (; ti != scene->triangles.end(); ti++) {
		glBegin(GL_LINE_LOOP);
		RenderTriangle(**ti);
		glEnd();
      }
    }
  }

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
