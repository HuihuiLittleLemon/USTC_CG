#pragma once

#include<QPoint>
#include<vector>

class WarpingMethod {
public:
	WarpingMethod();
	~WarpingMethod();
	virtual QPoint f_func(QPoint x) = 0; //pure virtual without realize should add "=0"
	int distance_square(QPoint point1, QPoint point2);

protected:
	std::vector<QPoint> points_p_;
	std::vector<QPoint> points_q_;
};

