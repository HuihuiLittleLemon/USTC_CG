#pragma once

#include "Shape.h"

class myEllipse : public Shape {
public:
	myEllipse();
	~myEllipse();

	void Draw(QPainter& painter)const override;
};

