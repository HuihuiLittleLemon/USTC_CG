#pragma once
#include "shape.h"
class myPolygon :public Shape {
public:
	std::vector<QPoint*> point_list_;
	bool drawComplete;

public:
	myPolygon();
	~myPolygon();
	void Draw(QPainter& painter) override;
	void find_points_on_the_edge(QPoint vertex_upper, QPoint vertex_lower, int** cur);
	void fill(QPainter& painter);

	typedef struct {
		int start_y;
		int end_y;
		bool in_scan_list;
		int **cur_x;
	}edges;
};

