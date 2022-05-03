#pragma once

#include "WarpingMethod.h"
#include <QPair>
#include <iostream>

class IDW :public WarpingMethod {
public:
	IDW(std::vector<QPoint> points_p, std::vector<QPoint> points_q, bool use_default_matrix_D = true);
	~IDW();
	QPoint f_func(QPoint x);

private:
	float* w_i_;
	float** D_i_;
	bool use_default_matrix_D_;						// default D equal identity matrix
private:
	QPair<float, float> fi_func(QPoint x, int i);	//use QPair<float,float> as type of return value to guaranteed precision
	void w_func(QPoint x);
	void cal_matrix_D();
};

