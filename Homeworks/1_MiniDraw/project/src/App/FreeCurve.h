#pragma once

#include "shape.h"

class FreeCurve : public Shape {
public:
	std::vector<QPoint*> ponit_list_;
public:
	FreeCurve();
	~FreeCurve();

	void Draw(QPainter& painter) const override;
};



