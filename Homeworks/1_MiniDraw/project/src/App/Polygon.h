#pragma once
#include "shape.h"
class myPolygon :public Shape {
public:
	std::vector<QPoint*> ponit_list_;
	bool drawComplete;

public:
	myPolygon();
	~myPolygon();
	void Draw(QPainter& painter)const override;
};

