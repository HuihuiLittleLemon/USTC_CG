#pragma once

#include "shape.h"

class FreeHand : public Shape {
public:
	std::vector<QPoint*> point_list_;
public:
	FreeHand();
	~FreeHand();

	void Draw(QPainter& painter) override;
};



