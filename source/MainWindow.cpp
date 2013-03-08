#include "MainWindow.h"
#include "BasicRenderer.h"
#include "CentralWidget.h"

MainWindow::MainWindow(Scene *s, QWidget *parent) : QMainWindow(parent)
{
	CentralWidget * cw = new CentralWidget(s, this);

	setCentralWidget(cw);
}


MainWindow::~MainWindow(void)
{
}
