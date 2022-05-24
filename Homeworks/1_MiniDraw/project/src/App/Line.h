#pragma once

#include "Shape.h"

class Line : public Shape {
public:
	Line();
	~Line();

	void Draw(QPainter& painter) override;//draw line with Bresenham algorithm
};
