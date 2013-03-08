#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <qmainwindow.h>
#include "Scene.h"

class MainWindow :
	public QMainWindow
{
public:
	MainWindow(Scene *s, QWidget *parent = 0);
	~MainWindow(void);
};

#endif

