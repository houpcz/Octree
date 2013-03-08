#pragma once
#include <qwidget.h>
#include "Scene.h"

class CentralWidget :
	public QWidget
{
public:
	CentralWidget(Scene *s, QWidget *parent = 0);
	virtual ~CentralWidget(void);
};

