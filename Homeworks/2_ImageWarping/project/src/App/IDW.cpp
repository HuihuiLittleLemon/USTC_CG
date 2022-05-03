#include "IDW.h"
#include <Eigen/Dense>
#include <Eigen/Dense>

using namespace Eigen;

IDW::IDW(std::vector<QPoint>  points_p, std::vector<QPoint>  points_q, bool use_default_matrix_D) {
	points_p_ = points_p;
	points_q_ = points_q;
	use_default_matrix_D_ = use_default_matrix_D;
	w_i_ = new float[points_p.size()];
	if (use_default_matrix_D_ == false) {
		D_i_ = new float*[points_p_.size()];
		for (int i = 0; i < points_p_.size(); i++) {
			D_i_[i] = new float[4];
		}
		cal_matrix_D();
	}
}

IDW::~IDW() {
	delete w_i_;

	if (use_default_matrix_D_ == false) {
		for (int i = 0; i < points_p_.size(); i++) {
			delete D_i_[i];
		}
		delete[] D_i_;
	}
}

void IDW::cal_matrix_D() {
	float delta_p_x, delta_p_y, delta_q_x, delta_q_y;
	float sigma = 0.0f;
	MatrixXf A(2, 2);//elements of matrix A,,D,B, solve A*D=B to get each D for i
	MatrixXf B(2, 2);

	if (points_p_.size() == 1) {
		D_i_[0][0] = 1;
		D_i_[0][1] = 0;
		D_i_[0][2] = 0;
		D_i_[0][3] = 1;
		return;
	}

	for (int i = 0; i < points_p_.size(); i++) {
		//set A=0,B=0; 
		for (int row = 0; row < 2; row++) {
			for (int col = 0; col < 2; col++) {
				A(row, col) = 0.0f;
				B(row, col) = 0.0f;
			}
		}

		for (int j = 0; j < points_p_.size(); j++) {
			if (j == i) {
				continue;
			}
			sigma = 1.0 / distance_square(points_p_[i], points_p_[j]);
			delta_p_x = points_p_[j].x() - points_p_[i].x();
			delta_p_y = points_p_[j].y() - points_p_[i].y();
			delta_q_x = points_q_[j].x() - points_q_[i].x();
			delta_q_y = points_q_[j].y() - points_q_[i].y();

			A(0, 0) += sigma * delta_p_x * delta_p_x;
			A(0, 1) += sigma * delta_p_y * delta_p_x;
			A(1, 0) = A(0, 1);
			A(1, 1) += sigma * delta_p_y * delta_p_y;

			B(0, 0) += sigma * delta_p_x * delta_q_x;
			B(0, 1) += sigma * delta_p_x * delta_q_y;
			B(1, 0) += sigma * delta_p_y * delta_q_x;
			B(1, 1) += sigma * delta_p_y * delta_q_y;
		}
		// if detA=0,A*D=B is unsolvableset set D=I
		if (abs(A.determinant()) < 1e-6) {
			D_i_[i][0] = 1;
			D_i_[i][1] = 0;
			D_i_[i][2] = 0;
			D_i_[i][3] = 1;
			continue;
		}
		//std::cout << "A:" << std::endl;
		//std::cout << A << std::endl;
		//std::cout << "B:" << std::endl;
		//std::cout << B << std::endl;
		MatrixXf D = (A.colPivHouseholderQr().solve(B)).transpose();
		//std::cout << "D:" << std::endl;
		//std::cout << D << std::endl;
		D_i_[i][0] = D(0, 0);
		D_i_[i][1] = D(0, 1);
		D_i_[i][2] = D(1, 0);
		D_i_[i][3] = D(1, 1);
	}
}

QPoint IDW::f_func(QPoint x) {
	QPoint result(0,0);
	QPair<float, float> tmp;
	float result_x = 0;
	float result_y = 0;
	w_func(x);
	for (int i = 0; i < points_p_.size(); i++) {
		tmp = fi_func(x, i);
		result_x += w_i_[i] * tmp.first;
		result_y += w_i_[i] * tmp.second;
	}
	return QPoint(round(result_x), round(result_y));
}

QPair<float, float> IDW::fi_func(QPoint x, int i) {
	float temp_point_x;
	float temp_point_y;

	if (use_default_matrix_D_ == true) {
		temp_point_x = points_q_[i].x() + x.x() - points_p_[i].x();
		temp_point_y = points_q_[i].y() + x.y() - points_p_[i].y();
	}
	else {
		temp_point_x = points_q_[i].x() + D_i_[i][0] * (x.x() - points_p_[i].x()) + D_i_[i][1] * (x.y() - points_p_[i].y());
		temp_point_y = points_q_[i].y() + D_i_[i][2] * (x.x() - points_p_[i].x()) + D_i_[i][3] * (x.y() - points_p_[i].y());
	}

	return QPair(temp_point_x, temp_point_y);

}

void IDW::w_func(QPoint x) {
	float sum = 0.0f;
	float dis;

	for (int i = 0; i < points_p_.size(); i++) {
		dis = distance_square(x, points_p_[i]);
		if(dis == 0 ){
			w_i_[i] = 1;
			for (int j = 0; j < points_p_.size(); j++) {
				if (j != i) {
					w_i_[j] = 0;
				}
			}
			return;
		}
		w_i_[i] = 1.0 / dis;
		sum = sum + w_i_[i];
	}

	for (int i = 0; i < points_p_.size(); i++) {

		w_i_[i] = w_i_[i] / sum;
	}
}
