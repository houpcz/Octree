// ===================================================================
// $Id: RendererWidget.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $


#include "GL\glut.h"

// Qt headers
#include <QtGui>
#include <QtOpenGL>

// C++ standard headers
#include <cmath>

// local headers
#include "common.h"
#include "RendererWidget.h"
#include "Trackball.h"
RendererWidget::RendererWidget(QWidget *parent)
  : QGLWidget(parent)
{
  xRot = 0;
  yRot = 0;
  zRot = 0;
  scale = 1.0f;
  mWireframe = false;
  trackball(manipulatorLastQuat, 0.0f, 0.0f, 0.0f, 0.0f);

}

RendererWidget::~RendererWidget()
{
  makeCurrent();
}

QSize
RendererWidget::minimumSizeHint() const
{
  return QSize(100, 100);
}

QSize
RendererWidget::sizeHint() const
{
  // Here is the standard size of the window in pixels
  return QSize(800, 600);
}

void
RendererWidget::setXRotation(int angle)
{
  normalizeAngle(&angle);
  if (angle != xRot) {
    xRot = angle;
    updateGL();
  }
}

void
RendererWidget::setYRotation(int angle)
{
  normalizeAngle(&angle);
  if (angle != yRot) {
    yRot = angle;
    updateGL();
  }
}

void
RendererWidget::setZRotation(int angle)
{
  normalizeAngle(&angle);
  if (angle != zRot) {
    zRot = angle;
    updateGL();
  }
}

void
RendererWidget::initializeGL()
{  
  const char *vendorString, *rendererString, *versionString, *extString;

  // get GL info
  vendorString = (const char *) glGetString(GL_VENDOR);
  rendererString = (const char *) glGetString(GL_RENDERER);
  versionString = (const char *) glGetString(GL_VERSION);
  extString = (const char *) glGetString(GL_EXTENSIONS);
  
  Debug << "Info: GL Vendor = "<<vendorString<<endl
        << "Info: GL Renderer = "<<rendererString<<endl
        << "Info: GL Version = "<<versionString<<endl
        << "Info: GL Extensions = "<<extString<<endl<<flush;

  GLboolean stereo;
  // check if the STEREO is supported
  glGetBooleanv(GL_STEREO, &stereo);
  if (stereo)
    Debug<<"Info: GL STEREO is supported.\n";
  else
    Debug<<"Info: GL STEREO NOT supported.\n";

  return;
}

void
RendererWidget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  float m[4][4];

  glLoadIdentity();
  gluLookAt(0.0, 0.0, 1.0,  /* eye is at (0,0,30) */
			0.0, 0.0, 0.0,      /* center is at (0,0,0) */
			0.0, 1.0, 0.0);      /* up is in positive Y direction */
  
  build_rotmatrix(m, manipulatorLastQuat);
  glMultMatrixf(&m[0][0]);
  
  
  //  glTranslated(0.0, 0.0, -1.0);
  //  glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
  //  glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
  //  glRotated(zRot / 16.0, 0.0, 0.0, 1.0);

  glScalef(scale, scale, scale);

  RenderScene();
}

void
RendererWidget::resizeGL(int width, int height)
{
  glViewport(0, 0, width, height);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(70.0, 1.0, 0.1, 100.0);

  GLdouble projMatrix[16];
  GLdouble modelMatrix[16];
  glGetDoublev(GL_PROJECTION, projMatrix);
  glGetDoublev(GL_MODELVIEW, modelMatrix);

  //  glOrtho(-0.5*width/height, +0.5*width/height, 0.5, -0.5, 4.0, 15.0);
  glMatrixMode(GL_MODELVIEW);
}


void
RendererWidget::mousePressEvent(QMouseEvent *event)
{
  lastPos = event->pos();
}

void
RendererWidget::keyPressEvent ( QKeyEvent * e )
{
  switch (e->key()) {
  case Qt::Key_W:
	mWireframe  = !mWireframe;
	updateGL();
  default:
	e->ignore();
	break;
  }
}

void
RendererWidget::mouseMoveEvent(QMouseEvent *event)
{
  int dx = event->x() - lastPos.x();
  int dy = event->y() - lastPos.y();  
  
  if (event->modifiers() & Qt::CTRL)
  {
    scale = scale*(1.0f - dy/(float)height());
    if (scale < 0.01)
      scale = 0.01;
    updateGL();
  }
  else {
	float W = width();
	float H = height();
	int x = event->x();
	int lastX = lastPos.x();
	int y = event->y();
	int lastY = lastPos.y();
	
	float quat[4];
	trackball(quat,
			  (2.0 * lastX - W) / W,
			  (H - 2.0 * lastY) / H,
			  (2.0 * x - W) / W,
			  (H - 2.0 * y) / H
			  );
	add_quats(quat, manipulatorLastQuat, manipulatorLastQuat);
  }
  


//   if (event->buttons() & Qt::LeftButton) {
//       setXRotation(xRot + 8 * dy);
//       setYRotation(yRot + 8 * dx);
//     }
//     else if (event->buttons() & Qt::RightButton) {
//       setXRotation(xRot + 8 * dy);
//       setZRotation(zRot + 8 * dx);
//     }
//   }
  lastPos = event->pos();
}


void
RendererWidget::normalizeAngle(int *angle)
{
  while (*angle < 0)
    *angle += 360 * 16;
  while (*angle > 360 * 16)
    *angle -= 360 * 16;
}

// End of file RendererWidget.cpp
