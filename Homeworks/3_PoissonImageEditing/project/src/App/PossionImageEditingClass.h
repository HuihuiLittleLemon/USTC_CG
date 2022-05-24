#pragma once
#include "Polygon.h"

#include <opencv2/core/core.hpp>
#include <QPoint>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseCholesky>
#include <Eigen/SparseLU>

using namespace Eigen;

class PossionImageEditingClass {
public:
	PossionImageEditingClass(QPoint src_start_point, QPoint src_end_point);
	~PossionImageEditingClass();
	void clone_with_importing_gradients_for_rectangle_region(QPoint dst_start_point, const cv::Mat& src_image_mat, cv::Mat& dst_image_mat, bool mixed);
	ComputationInfo preprocess_solver_sparse_for_rectangle_region();

	void get_inner_point_map_(std::vector<QPoint*> point_list_);
	void find_points_on_polygon_region_edges(QPoint vertex_upper, QPoint vertex_lower, int** cur);
	void clone_with_importing_gradients_for_polygon_region(QPoint dst_start_point, const cv::Mat& src_image_mat, cv::Mat& dst_image_mat, bool mixed);
	ComputationInfo preprocess_solver_sparse_for_polygon_region(myPolygon *polygon);

private:
	void color_channel_value_rectify(float& channel);

private:
	QPoint		src_start_point_;			// Left top point of rectangle region
	QPoint		src_end_point_;				// Right bottom point of rectangle region
	//LeastSquaresConjugateGradient<SparseMatrix<float>> solver_sparse_;
	//SimplicialLDLT<SparseMatrix<float>> solver_sparse_;
	//SparseLU<SparseMatrix<float>> solver_sparse_;
	SimplicialLLT<SparseMatrix<float>> solver_sparse_;
	SparseMatrix<float> A_saprse_;

	typedef struct {
		int start_y;
		int end_y;
		bool in_scan_list;
		int** cur_x;
	}edges;
	int edges_min_x, edges_min_y, edges_max_x, edges_max_y;
	std::map<unsigned int, int> inner_point_map_; // inner point position -> it's colum in A
};
