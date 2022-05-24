#include "PossionImageEditingClass.h"
#include <iostream>

#define GET_KEY(a,b)			(((a)<<16) + (b))
#define GET_X(key)				((key)>>16) 
#define GET_Y(key)				((key)&0xffff)
#define GET_KEY_UP(key)			GET_KEY(GET_X(key),GET_Y(key)-1)
#define GET_KEY_DOWN(key)		GET_KEY(GET_X(key),GET_Y(key)+1)
#define GET_KEY_LEFT(key)		GET_KEY(GET_X(key)-1,GET_Y(key))
#define GET_KEY_RIGHT(key)		GET_KEY(GET_X(key)+1,GET_Y(key))

PossionImageEditingClass::PossionImageEditingClass(QPoint src_start_point, QPoint src_end_point) {
	src_start_point_ = src_start_point;
	src_end_point_ = src_end_point;
}

PossionImageEditingClass::~PossionImageEditingClass() {

}

ComputationInfo PossionImageEditingClass::preprocess_solver_sparse_for_rectangle_region() {
	// Start point in source image
	int xsourcepos = src_start_point_.x();
	int ysourcepos = src_start_point_.y();
	// Width and Height of inside rectangle region 
	int w = src_end_point_.x() - src_start_point_.x() - 1;
	int h = src_end_point_.y() - src_start_point_.y() - 1;
	int pixel_num = w * h;
	int tmp;
	A_saprse_.resize(pixel_num, pixel_num);
	std::vector<Triplet<double>> tripletlist;

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			tmp = i * w + j;
			if (i == 0) {
				if (j == 0) {
					tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
					tripletlist.push_back(Triplet<double>(tmp, tmp + w, -1));
					tripletlist.push_back(Triplet<double>(tmp, tmp + 1, -1));
					continue;
				}
				else if (j == w - 1) {
					tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
					tripletlist.push_back(Triplet<double>(tmp, tmp + w, -1));
					tripletlist.push_back(Triplet<double>(tmp, tmp - 1, -1));
					continue;
				}
				tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
				tripletlist.push_back(Triplet<double>(tmp, tmp + w, -1));
				tripletlist.push_back(Triplet<double>(tmp, tmp - 1, -1));
				tripletlist.push_back(Triplet<double>(tmp, tmp + 1, -1));
				continue;
			}

			if (i == h - 1) {
				if (j == 0) {
					tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
					tripletlist.push_back(Triplet<double>(tmp, tmp - w, -1));
					tripletlist.push_back(Triplet<double>(tmp, tmp + 1, -1));
					continue;
				}
				else if (j == w - 1) {
					tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
					tripletlist.push_back(Triplet<double>(tmp, tmp - w, -1));
					tripletlist.push_back(Triplet<double>(tmp, tmp - 1, -1));
					continue;
				}
				tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
				tripletlist.push_back(Triplet<double>(tmp, tmp - w, -1));
				tripletlist.push_back(Triplet<double>(tmp, tmp - 1, -1));
				tripletlist.push_back(Triplet<double>(tmp, tmp + 1, -1));
				continue;
			}

			if (j == 0) {
				tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
				tripletlist.push_back(Triplet<double>(tmp, tmp - w, -1));
				tripletlist.push_back(Triplet<double>(tmp, tmp + w, -1));
				tripletlist.push_back(Triplet<double>(tmp, tmp + 1, -1));
				continue;
			}

			if (j == w - 1) {
				tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
				tripletlist.push_back(Triplet<double>(tmp, tmp - w, -1));
				tripletlist.push_back(Triplet<double>(tmp, tmp + w, -1));
				tripletlist.push_back(Triplet<double>(tmp, tmp - 1, -1));
				continue;
			}

			tripletlist.push_back(Triplet<double>(tmp, tmp, 4));
			tripletlist.push_back(Triplet<double>(tmp, tmp - w, -1));
			tripletlist.push_back(Triplet<double>(tmp, tmp + w, -1));
			tripletlist.push_back(Triplet<double>(tmp, tmp - 1, -1));
			tripletlist.push_back(Triplet<double>(tmp, tmp + 1, -1));
		}
	}
	A_saprse_.setFromTriplets(tripletlist.begin(), tripletlist.end());
	//std::cout << A_saprse_ << "\n";
	A_saprse_.uncompress();
	//A_saprse_.makeCompressed();
	//solver_sparse_.setTolerance(1e-6);
	solver_sparse_.compute(A_saprse_);
	std::cout << solver_sparse_.info() << "\n";

	return solver_sparse_.info();
}

void PossionImageEditingClass::clone_with_importing_gradients_for_rectangle_region(QPoint dst_start_point, const cv::Mat& src_image_mat, cv::Mat& dst_image_mat, bool mixed) {
	// Start point in object image
	int xpos = dst_start_point.x();
	int ypos = dst_start_point.y();

	// Start point in source image
	int xsourcepos = src_start_point_.x();
	int ysourcepos = src_start_point_.y();

	// Width and Height of inside rectangle region
	int w = src_end_point_.x() - src_start_point_.x() - 1;
	int h = src_end_point_.y() - src_start_point_.y() - 1;


	// Paste
	if (xpos <= 0 || ypos <= 0 || (xpos + w + 2 >= dst_image_mat.cols) || (ypos + h + 2 >= dst_image_mat.rows)) {
		return;
	}

	int pixel_num = w * h;
	int tmp;
	int vpq[3];//red green blue
	int src_left_dirivate, dst_left_dirivate, left_dirivate;
	int src_right_dirivate, dst_right_dirivate, right_dirivate;
	int src_up_dirivate, dst_up_dirivate, up_dirivate;
	int src_down_dirivate, dst_down_dirivate, down_dirivate;

	VectorXf b_red(pixel_num);
	VectorXf b_green(pixel_num);
	VectorXf b_blue(pixel_num);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			tmp = i * w + j;

			if (mixed == false) {
				for (int k = 0; k < 3; k++) {
					vpq[k] = 4 * src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j + 1)[k] - src_image_mat.at<cv::Vec3b>(ysourcepos + i, xsourcepos + j + 1)[k]
					- src_image_mat.at<cv::Vec3b>(ysourcepos + i + 2, xsourcepos + j + 1)[k] - src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j)[k]
					- src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j + 2)[k];
				}
			}
			else {		
				for (int k = 0; k < 3; k++) {
					src_left_dirivate = src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j + 1)[k] - src_image_mat.at<cv::Vec3b>(ysourcepos + i, xsourcepos + j + 1)[k];
					dst_left_dirivate = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 1)[k] - dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[k];
					left_dirivate = abs(src_left_dirivate) > abs(dst_left_dirivate) ? src_left_dirivate : dst_left_dirivate;

					src_right_dirivate = src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j + 1)[k] - src_image_mat.at<cv::Vec3b>(ysourcepos + i + 2, xsourcepos + j + 1)[k];
					dst_right_dirivate = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 1)[k] - dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[k];
					right_dirivate = abs(src_right_dirivate) > abs(dst_right_dirivate) ? src_right_dirivate : dst_right_dirivate;

					src_up_dirivate = src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j + 1)[k] - src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j)[k];
					dst_up_dirivate = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 1)[k] - dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[k];
					up_dirivate = abs(src_up_dirivate) > abs(dst_up_dirivate) ? src_up_dirivate : dst_up_dirivate;

					src_down_dirivate = src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j + 1)[k] - src_image_mat.at<cv::Vec3b>(ysourcepos + i + 1, xsourcepos + j + 2)[k];
					dst_down_dirivate = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 1)[k] - dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[k];
					down_dirivate = abs(src_down_dirivate) > abs(dst_down_dirivate) ? src_down_dirivate : dst_down_dirivate;
					
					vpq[k] = left_dirivate + right_dirivate + up_dirivate + down_dirivate;
				}	
			}

			if (i == 0) {
				if (j == 0) {
					b_red(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[0] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[0];
					b_green(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[1] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[1];
					b_blue(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[2] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[2];
				}
				else if (j == w - 1) {
					b_red(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[0] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[0];
					b_green(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[1] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[1];
					b_blue(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[2] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[2];
				}
				else {
					b_red(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[0];
					b_green(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[1];
					b_blue(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i, xpos + j + 1)[2];
				}
				b_red(tmp) += vpq[0];
				b_green(tmp) += vpq[1];
				b_blue(tmp) += vpq[2];
				continue;
			}

			if (i == h - 1) {
				if (j == 0) {
					b_red(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[0] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[0];
					b_green(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[1] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[1];
					b_blue(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[2] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[2];
				}
				else if (j == w - 1) {
					b_red(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[0] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[0];
					b_green(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[1] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[1];
					b_blue(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[2] + dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[2];
				}
				else {
					b_red(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[0];
					b_green(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[1];
					b_blue(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 2, xpos + j + 1)[2];
				}
				b_red(tmp) += vpq[0];
				b_green(tmp) += vpq[1];
				b_blue(tmp) += vpq[2];
				continue;
			}

			if (j == 0) {
				b_red(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[0] + vpq[0];
				b_green(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[1] + vpq[1];
				b_blue(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j)[2] + vpq[2];
				continue;
			}

			if (j == w - 1) {
				b_red(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[0] + vpq[0];
				b_green(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[1] + vpq[1];
				b_blue(tmp) = dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 2)[2] + vpq[2];
				continue;
			}

			b_red(tmp) = vpq[0];
			b_green(tmp) = vpq[1];
			b_blue(tmp) = vpq[2];
		}
	}

	VectorXf x_0 = solver_sparse_.solve(b_red);
	VectorXf x_1 = solver_sparse_.solve(b_green);
	VectorXf x_2 = solver_sparse_.solve(b_blue);
	//std::cout << b_red << "\n";
	//std::cout <<"-------------------" << "\n";
	//std::cout << x_0 << "\n";
	//std::cout << "-------------------" << "\n";
	//std::cout << x_2 << "\n";

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			color_channel_value_rectify(x_0(w * i + j));
			color_channel_value_rectify(x_1(w* i + j));
			color_channel_value_rectify(x_2(w* i + j));

			dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 1)[0] = round(x_0(w * i + j));
			dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 1)[1] = round(x_1(w * i + j));
			dst_image_mat.at<cv::Vec3b>(ypos + i + 1, xpos + j + 1)[2] = round(x_2(w * i + j));
		}
	}
}

//correct the wrong R,G,B color value 
void PossionImageEditingClass::color_channel_value_rectify(float& channel) {
	if (channel < 0) {
		channel = 0;
	}
		
	if (channel > 255) {
		channel = 255;
	}
		
}

void PossionImageEditingClass::find_points_on_polygon_region_edges(QPoint vertex_upper, QPoint vertex_lower, int** cur) {
	//line parallel to y axis
	int dx = vertex_lower.x() - vertex_upper.x();
	int dy = vertex_lower.y() - vertex_upper.y();

	//slope > 1 flag 
	bool slope_over_1 = false;
	int quartile = 1;
	int tmp;

	if (dx == 0) {

		for (int i = 0; i <= vertex_lower.y() - vertex_upper.y(); i++) {
			cur[i][0] = vertex_upper.x();
			cur[i][1] = vertex_upper.x();
		}
		return;
	}

	if (dy == 0) {
		if (dx >= 0) {
			cur[0][0] = vertex_upper.x();
			cur[0][1] = vertex_lower.x();
		}
		else {
			cur[0][0] = vertex_lower.x();
			cur[0][1] = vertex_upper.x();
		}

		return;
	}

	//transform second,thead,forth quartile line to first quartile.
	if (dx < 0 && dy > 0) {
		dx = -dx;
		quartile = 2;
	}

	//transform line in first quartile with slope > 1 to slope < 1
	if (dx < dy) {
		slope_over_1 = true;
		tmp = dx;
		dx = dy;
		dy = tmp;
	}

	//draw line with Brsenham algorithm
	int points_num = dx + 1;
	cur[0][0] = vertex_upper.x();
	cur[0][1] = vertex_upper.x();
	int incre_y = 0;
	int  incre_Northeast = 2 * (dy - dx);
	int  incre_East = 2 * dy;
	int P_i = 2 * dy - dx;
	int i = 0;

	for (i = 1; i < points_num; i++) {
		if (P_i >= 0) {
			incre_y++;
			P_i += incre_Northeast;
			if (slope_over_1 == false) {
				if (quartile == 1) {
					cur[incre_y][0] = vertex_upper.x() + i;
					cur[incre_y][1] = vertex_upper.x() + i;
				}
				else {
					cur[incre_y][0] = vertex_upper.x() - i;
					cur[incre_y][1] = vertex_upper.x() - i;
				}
			}
			else
			{
				if (quartile == 1) {
					cur[i][0] = vertex_upper.x() + incre_y;
					cur[i][1] = vertex_upper.x() + incre_y;
				}
				else {
					cur[i][0] = vertex_upper.x() - incre_y;
					cur[i][1] = vertex_upper.x() - incre_y;
				}
			}
		}
		else {
			P_i += incre_East;
			if (slope_over_1 == false) {
				if (quartile == 1) {
					cur[incre_y][1] = vertex_upper.x() + i;
				}
				else {
					cur[incre_y][0] = vertex_upper.x() - i;
				}
			}
			else
			{
				if (quartile == 1) {
					cur[i][0] = vertex_upper.x() + incre_y;
					cur[i][1] = vertex_upper.x() + incre_y;
				}
				else {
					cur[i][0] = vertex_upper.x() - incre_y;
					cur[i][1] = vertex_upper.x() - incre_y;
				}
			}
		}
	}
}

void PossionImageEditingClass::get_inner_point_map_(std::vector<QPoint*> point_list_) {
	std::vector<edges> edges_list;
	edges polygon_edges;
	int num;
	int inner_points_code = 0;
	QPoint current_vertex = src_start_point_, next_vertex = src_start_point_;
	
	if (point_list_.size() != 0) {
		edges_min_x = edges_max_x = current_vertex.x();
		edges_min_y = edges_max_y = current_vertex.y();

		for (size_t i = 0; i <= point_list_.size() + 1; i++) {
			if (i == point_list_.size()) {
				current_vertex = next_vertex;
				next_vertex = src_end_point_;

			}
			else if (i == point_list_.size() + 1) {
				current_vertex = src_start_point_;
				next_vertex = src_end_point_;
			}
			else {
				current_vertex = next_vertex;
				next_vertex = *point_list_[i];
			}

			if (current_vertex == next_vertex) {
				continue;
			}

			if (current_vertex.y() <= next_vertex.y()) {
				polygon_edges.start_y = current_vertex.y();
				polygon_edges.end_y = next_vertex.y();
				num = polygon_edges.end_y - polygon_edges.start_y + 1;
				polygon_edges.cur_x = new int* [num];
				for (int j = 0; j < num; j++) {
					polygon_edges.cur_x[j] = new int[2];
				}
				find_points_on_polygon_region_edges(current_vertex, next_vertex, polygon_edges.cur_x);
			}
			else {
				polygon_edges.start_y = next_vertex.y();
				polygon_edges.end_y = current_vertex.y();
				num = polygon_edges.end_y - polygon_edges.start_y + 1;
				polygon_edges.cur_x = new int* [num];
				for (int j = 0; j < num; j++) {
					polygon_edges.cur_x[j] = new int[2];
				}
				find_points_on_polygon_region_edges(next_vertex, current_vertex, polygon_edges.cur_x);
			}

			edges_list.push_back(polygon_edges);

			if (next_vertex.x() > edges_max_x) edges_max_x = next_vertex.x();
			if (next_vertex.x() < edges_min_x) edges_min_x = next_vertex.x();
			if (next_vertex.y() > edges_max_y) edges_max_y = next_vertex.y();
			if (next_vertex.y() < edges_min_y) edges_min_y = next_vertex.y();
		}

		std::vector<int> x_list;
		for (size_t i = 0; i < edges_list.size(); i++) {

			edges_list[i].in_scan_list = false;
		}

		for (int i = edges_min_y; i < edges_max_y; i++) {

			for (size_t k = 0; k < edges_list.size(); k++) {
				if (edges_list[k].start_y == i) {
					edges_list[k].in_scan_list = true;
				}
				if (edges_list[k].end_y == i) {
					edges_list[k].in_scan_list = false;
				}

				if (edges_list[k].in_scan_list == true) {
					x_list.push_back(edges_list[k].cur_x[i - edges_list[k].start_y][0]);
				}
			}

			sort(x_list.begin(), x_list.end());

			for (int j = 0; j < x_list.size(); j++) {
				if (j % 2 == 0) {
					if (j + 1 < x_list.size()) {
						for (int k = x_list[j] + 1; k < x_list[j + 1]; k++) {
							inner_point_map_.insert(std::pair<unsigned int, int>((k << 16) + i, inner_points_code));
							inner_points_code++;
						}

					}
				}
			}
			x_list.clear();
		}
	}

	//delete dynamic memory
	for (size_t i = 0; i < edges_list.size(); i++) {
		for (int j = 0; j <= edges_list[i].end_y - edges_list[i].start_y; j++) {
			delete[]edges_list[i].cur_x[j];
		}
		delete[]edges_list[i].cur_x;
	}
}

ComputationInfo PossionImageEditingClass::preprocess_solver_sparse_for_polygon_region(myPolygon *polygon) {
	get_inner_point_map_(polygon->point_list_);

	int pixel_num = inner_point_map_.size();
	unsigned int key;
	A_saprse_.resize(pixel_num, pixel_num);
	std::vector<Triplet<double>> tripletlist;

	for (auto iter : inner_point_map_) {

		tripletlist.push_back(Triplet<double>(iter.second, iter.second, 4));
		key = GET_KEY_UP(iter.first);
		if (inner_point_map_.find(key) != inner_point_map_.end()) {
			tripletlist.push_back(Triplet<double>(iter.second, inner_point_map_[key], -1));
		}

		key = GET_KEY_DOWN(iter.first);
		if (inner_point_map_.find(key) != inner_point_map_.end()) {
			tripletlist.push_back(Triplet<double>(iter.second, inner_point_map_[key], -1));
		}

		key = GET_KEY_LEFT(iter.first);
		if (inner_point_map_.find(key) != inner_point_map_.end()) {
			tripletlist.push_back(Triplet<double>(iter.second, inner_point_map_[key], -1));
		}

		key = GET_KEY_RIGHT(iter.first);
		if (inner_point_map_.find(key) != inner_point_map_.end()) {
			tripletlist.push_back(Triplet<double>(iter.second, inner_point_map_[key], -1));
		}
	}

	A_saprse_.setFromTriplets(tripletlist.begin(), tripletlist.end());
	//std::cout << A_saprse_ << "\n";
	A_saprse_.uncompress();
	//A_saprse_.makeCompressed();
	//solver_sparse_.setTolerance(1e-6);
	solver_sparse_.compute(A_saprse_);
	std::cout << solver_sparse_.info() << "\n";

	return solver_sparse_.info();
}

void PossionImageEditingClass::clone_with_importing_gradients_for_polygon_region(QPoint dst_start_point, const cv::Mat& src_image_mat, cv::Mat& dst_image_mat, bool mixed) {
	// Start point in object image
	int xpos = dst_start_point.x();
	int ypos = dst_start_point.y();

	// Start point in source image
	int xsourcepos = src_start_point_.x();
	int ysourcepos = src_start_point_.y();

	// Paste
	if ((edges_min_x - xsourcepos + xpos <= 0) || (edges_max_x - xsourcepos + xpos >= dst_image_mat.cols)
		|| (edges_min_y - ysourcepos + ypos <= 5) || (edges_max_y - ysourcepos + ypos >= dst_image_mat.rows)) {
		return;
	}

	int pixel_num = inner_point_map_.size();
	unsigned int key;
	int vpq[3];//red green blue
	int src_left_dirivate, dst_left_dirivate, left_dirivate;
	int src_right_dirivate, dst_right_dirivate, right_dirivate;
	int src_up_dirivate, dst_up_dirivate, up_dirivate;
	int src_down_dirivate, dst_down_dirivate, down_dirivate;

	VectorXf b_red(pixel_num);
	VectorXf b_green(pixel_num);
	VectorXf b_blue(pixel_num);

	for (auto iter : inner_point_map_) {
		b_red(iter.second) = 0;
		b_green(iter.second) = 0;
		b_blue(iter.second) = 0;

		if (mixed == false) {
			for (int k = 0; k < 3; k++) {
				vpq[k] = 4 * src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first))[k] - src_image_mat.at<cv::Vec3b>(GET_Y(iter.first) - 1, GET_X(iter.first))[k]
					- src_image_mat.at<cv::Vec3b>(GET_Y(iter.first) + 1, GET_X(iter.first))[k] - src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first) - 1)[k]
					- src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first) + 1)[k];
			}
		}
		else {
			for (int k = 0; k < 3; k++) {
				src_up_dirivate = src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first))[k] - src_image_mat.at<cv::Vec3b>(GET_Y(iter.first) - 1, GET_X(iter.first))[k];
				dst_up_dirivate = dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos)[k] - dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos - 1, xpos + GET_X(iter.first) - xsourcepos)[k];
				up_dirivate =  abs(src_up_dirivate) > abs(dst_up_dirivate) ? src_up_dirivate : dst_up_dirivate;

				src_down_dirivate = src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first))[k] - src_image_mat.at<cv::Vec3b>(GET_Y(iter.first) + 1, GET_X(iter.first))[k];
				dst_down_dirivate = dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos)[k] - dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos + 1, xpos + GET_X(iter.first) - xsourcepos)[k];
				down_dirivate = abs(src_down_dirivate) > abs(dst_down_dirivate) ? src_down_dirivate : dst_down_dirivate;

				src_left_dirivate = src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first))[k] - src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first) - 1)[k];
				dst_left_dirivate = dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos)[k] - dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos - 1)[k];
				left_dirivate = abs(src_left_dirivate) > abs(dst_left_dirivate) ? src_left_dirivate : dst_left_dirivate;

				src_right_dirivate = src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first))[k] - src_image_mat.at<cv::Vec3b>(GET_Y(iter.first), GET_X(iter.first) + 1)[k];
				dst_right_dirivate = dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos)[k] - dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos + 1)[k];
				right_dirivate= abs(src_right_dirivate) > abs(dst_right_dirivate) ? src_right_dirivate : dst_right_dirivate;

				vpq[k] = left_dirivate + right_dirivate + up_dirivate + down_dirivate;
			}
		}

		key = GET_KEY_UP(iter.first);
		if (inner_point_map_.find(key) == inner_point_map_.end()) {
			b_red(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos - 1, xpos + GET_X(iter.first) - xsourcepos)[0];
			b_green(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos - 1, xpos + GET_X(iter.first) - xsourcepos)[1];
			b_blue(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos - 1, xpos + GET_X(iter.first) - xsourcepos)[2];
		}

		key = GET_KEY_DOWN(iter.first);
		if (inner_point_map_.find(key) == inner_point_map_.end()) {
			b_red(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos + 1, xpos + GET_X(iter.first) - xsourcepos)[0];
			b_green(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos + 1, xpos + GET_X(iter.first) - xsourcepos)[1];
			b_blue(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos + 1, xpos + GET_X(iter.first) - xsourcepos)[2];
		}

		key = GET_KEY_LEFT(iter.first);
		if (inner_point_map_.find(key) == inner_point_map_.end()) {
			b_red(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos - 1)[0];
			b_green(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos - 1)[1];
			b_blue(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos - 1)[2];
		}

		key = GET_KEY_RIGHT(iter.first);
		if (inner_point_map_.find(key) == inner_point_map_.end()) {
			b_red(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos + 1)[0];
			b_green(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos + 1)[1];
			b_blue(iter.second) += dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos + 1)[2];
		}

		b_red(iter.second) += vpq[0];
		b_green(iter.second) += vpq[1];
		b_blue(iter.second) += vpq[2];
	}

	VectorXf x_0 = solver_sparse_.solve(b_red);
	VectorXf x_1 = solver_sparse_.solve(b_green);
	VectorXf x_2 = solver_sparse_.solve(b_blue);
	//std::cout << b_red << "\n";
	//std::cout <<"-------------------" << "\n";
	//std::cout << x_0 << "\n";
	//std::cout << "-------------------" << "\n";
	//std::cout << x_2 << "\n";

	for (auto iter : inner_point_map_) {
		color_channel_value_rectify(x_0(iter.second));
		color_channel_value_rectify(x_1(iter.second));
		color_channel_value_rectify(x_2(iter.second));

		dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos)[0] = round(x_0(iter.second));
		dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos)[1] = round(x_1(iter.second));
		dst_image_mat.at<cv::Vec3b>(ypos + GET_Y(iter.first) - ysourcepos, xpos + GET_X(iter.first) - xsourcepos)[2] = round(x_2(iter.second));
	}
}