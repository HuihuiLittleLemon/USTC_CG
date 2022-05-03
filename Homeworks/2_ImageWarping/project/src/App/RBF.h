#pragma once

#include "WarpingMethod.h"
#include <Eigen/Dense>

using namespace Eigen;

class RBF :public WarpingMethod {
public:
	RBF(std::vector<QPoint> points_p, std::vector<QPoint> points_q, float c = 25, float u = 0.5);
	~RBF();
	QPoint f_func(QPoint x);
private:
	float** alpha_i_;
	int c_;
	float u_;
	MatrixXf D_;
private:

	float Rij(QPoint i, QPoint j);
	void cal_all_coff();	//solve all the coefficient:A,b,,alpha
};

