#include "CentralWidget.h"
#include <qboxlayout.h>
#include "BasicRenderer.h"
#include "RayRenderer.h"

CentralWidget::CentralWidget(Scene *s, QWidget *parent)
{
	 BasicRenderer * renderer = new BasicRenderer(s);
	 Scene * s2 = new Scene(*s);
	 RayRenderer * rayRenderer = new RayRenderer(s2);
     QHBoxLayout *layout = new QHBoxLayout;

	 layout->addWidget(renderer);
	 layout->addWidget(rayRenderer);
     

     setLayout(layout);
}


CentralWidget::~CentralWidget(void)
{
}
