#include "Polygon.h"

myPolygon::myPolygon() {

}

myPolygon::~myPolygon() {
	for (QPoint* iter : point_list_){
		delete iter;
	}
}

void myPolygon::Draw(QPainter& painter) {
	QPoint start_= start, end_ = start;
	//painter.setRenderHint(QPainter::Antialiasing);

	if (point_list_.size() != 0) {		
		for (size_t i = 0; i < point_list_.size(); i++) {
			start_ = end_;
			end_ = *point_list_[i];
			painter.drawLine(start_, end_);
		}
	}
	if (this->drawComplete == true)
	{
		painter.drawLine(start, end);
		fill(painter);
	}

	painter.drawLine(end_, end);

}

void myPolygon::find_points_on_the_edge(QPoint vertex_upper, QPoint vertex_lower,int** cur) {
	//line parallel to y axis
	int dx = vertex_lower.x() - vertex_upper.x();
	int dy = vertex_lower.y() - vertex_upper.y();

	//slope > 1 flag 
	bool slope_over_1 = false;
	int quartile = 1;

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
		myswap(dx, dy);
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
					cur[incre_y][0]= vertex_upper.x() + i;
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

	//if (slope_over_1 == true) { incre_y = i -1; }
	//for (int j = 0; j <= incre_y; j++) {
	//	qDebug() << "y = " << j + vertex_upper.y()<<"  x1 = "<< cur[j][0]<< ",x2 = " << cur[j][1];
	//}
}

void myPolygon::fill(QPainter& painter) {
	std::vector<edges> edges_list;
	edges polygon_edges;
	int num;
	QPoint current_vertex = start, next_vertex = start;

	if (point_list_.size() != 0) {
		for (size_t i = 0; i <= point_list_.size()+1; i++) {
			if (i == point_list_.size()) {
				current_vertex = next_vertex;
				next_vertex = end;
				
			}
			else if (i == point_list_.size() + 1) {
				current_vertex = start;
				next_vertex = end;
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
				find_points_on_the_edge(current_vertex, next_vertex, polygon_edges.cur_x);
			}
			else {
				polygon_edges.start_y = next_vertex.y();
				polygon_edges.end_y = current_vertex.y();
				num = polygon_edges.end_y - polygon_edges.start_y + 1;
				polygon_edges.cur_x = new int* [num];
				for (int j = 0; j < num; j++) {
					polygon_edges.cur_x[j] = new int[2];
				}
				find_points_on_the_edge(next_vertex, current_vertex, polygon_edges.cur_x);
			}

			edges_list.push_back(polygon_edges);
 		}

		std::vector<int> x_list;
		QPen pen;
		pen.setColor(Qt::blue);
		painter.setPen(pen);
		int max_y = edges_list[0].end_y;
		int min_y = edges_list[0].start_y;
		for (size_t i = 0; i < edges_list.size();i++ ) {
			if (edges_list[i].start_y < min_y) {
				min_y = edges_list[i].start_y;
			}

			if (edges_list[i].end_y > max_y) {
				max_y = edges_list[i].end_y;
			}

			edges_list[i].in_scan_list = false;
		}

		for (int i = min_y; i <= max_y; i++) {
		
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
							painter.drawPoint(k, i);
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

