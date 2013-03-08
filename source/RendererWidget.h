// ===================================================================
// $Id: RendererWidget.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006


#ifndef __RENDERER_WIDGET_H
#define __RENDERER_WIDGET_H

// This is a general widget for OpenGL renderer using Qt

#include <QGLWidget>

class RendererWidget :
  public QGLWidget
{
  Q_OBJECT

public:
  RendererWidget(QWidget *parent = 0);
  ~RendererWidget();
  
  QSize minimumSizeHint() const;
  QSize sizeHint() const;
  int xRotation() const { return xRot; }
  int yRotation() const { return yRot; }
  int zRotation() const { return zRot; }

  public slots:
  void setXRotation(int angle);
  void setYRotation(int angle);
  void setZRotation(int angle);
  
  // This method has to be rendefined by all renderer implementations
  virtual void RenderScene() = 0;
  
protected:
  
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void
  keyPressEvent ( QKeyEvent * e );

  bool mWireframe;

private:
  void normalizeAngle(int *angle);

  
  float manipulatorLastQuat[4];

  int xRot;
  int yRot;
  int zRot;

  float scale;
  QPoint lastPos;
};

#endif // __RENDERER_WIDGET_H

