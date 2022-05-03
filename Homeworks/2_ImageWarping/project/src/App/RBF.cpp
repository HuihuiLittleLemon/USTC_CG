#include"RBF.h"
#include<iostream>

RBF::RBF(std::vector<QPoint> points_p, std::vector<QPoint> points_q, float c, float u) {
	points_p_ = points_p;
	points_q_ = points_q;
	c_ = c;
	u_ = u;

	alpha_i_ = new float* [points_p_.size()];
	for (int i = 0; i < points_p_.size(); i++) {
		alpha_i_[i] = new float[2];
	}
}

RBF::~RBF() {
	for (int i = 0; i < points_p_.size(); i++) {
		delete alpha_i_[i];
	}
	delete[] alpha_i_;

}

void RBF::cal_all_coff() {
	int points_size = points_p_.size();
	MatrixXf A_(1, 1);
	MatrixXf B_(1, 1);
	A_.resize(points_size + 3, points_size + 3);
	B_.resize(points_size + 3, 2);
	D_.resize(points_size + 3, 2);
	for (int i = 0; i < points_size; i++) {
		for (int j = 0; j < points_size; j++) {
			A_(i, j) = Rij(points_p_[i], points_p_[j]);
		}
	}
	
	for (int i = 0; i < points_size; i++) {
		A_(points_size, i) = points_p_[i].x();
		A_(points_size + 1, i) = points_p_[i].y();
		A_(points_size + 2, i) = 1;

		A_(i, points_size) = points_p_[i].x();
		A_(i, points_size + 1) = points_p_[i].y();
		A_(i, points_size + 2) = 1;
	}

	for (int i = points_size; i < points_size + 3; i++) {
		for (int j = points_size; j < points_size + 3; j++) {
			A_(i, j) = 0.0f;
		}
	}

	//construct b
	for (int i = 0; i < points_size; i++) {
		B_(i, 0) = points_q_[i].x();
		B_(i, 1) = points_q_[i].y();
	}

	for (int i = points_size; i < points_size+3; i++) {
		B_(i, 0) = 0.0f;
		B_(i, 1) = 0.0f;
	}

	D_ = A_.colPivHouseholderQr().solve(B_);

	//std::cout << "D:" << std::endl;
	//std::cout << D_ << std::endl;
}

float RBF::Rij(QPoint i, QPoint j) {
	return pow(distance_square(i, j) + c_ * c_, u_);
}

QPoint RBF::f_func(QPoint x) {
	int points_size = points_p_.size();

	//translate when setting an anchor point
	if (points_size == 1) {
		return QPoint(x.x() + points_q_[0].x() - points_p_[0].x(), x.y() + points_q_[0].y() - points_p_[0].y());
	}

	MatrixXf X(1, 1);
	X.resize(1, points_size + 3);
	cal_all_coff();
	for (int i = 0; i < points_size; i++) {
		X(0, i) = Rij(x, points_p_[i]);
	}
	X(0, points_size) = x.x();
	X(0, points_size+1) = x.y();
	X(0, points_size + 2) = 1;

	MatrixXf f_x = X * D_;
	return QPoint(round(f_x(0,0)),round(f_x(0, 1)));
}