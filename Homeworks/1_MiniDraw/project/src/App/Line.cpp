#include "Line.h"

Line::Line() {

}

Line::~Line() {

}

//用Flag标识是哪一个区间的直线
//void Line::Draw(QPainter& painter)const {
//	painter.setRenderHint(QPainter::Antialiasing);
//	painter.drawLine(start, end);
//}

void  Line::Draw(QPainter& painter) {
	//line parallel to y axis
	int dx = end.x() - start.x();
	int dy = end.y() - start.y();

	//slope > 1 flag 
	bool slope_over_1 = false;
	int quartile = 1;

	//draw parallel lines to the y-axis
	if (dx == 0) {
		int begin_y, end_y;

		if (dy >= 0) {
			begin_y = start.y();
			end_y = end.y();
		}
		else {
			begin_y = end.y();
			end_y = start.y();
			dy = -dy;
		}

		for (int i = begin_y; i <= end_y; i++) {
			painter.drawPoint(start.x(), i);
		}
		return;
	}

	//draw parallel lines to the x-axis
	if (dy == 0) {
		int begin_x, end_x;

		if (dx >= 0) {
			begin_x = start.x();
			end_x = end.x();
		}
		else {
			begin_x = end.x();
			end_x = start.x();
			dx = -dx;
		}

		for (int i = begin_x; i <= end_x; i++) {
			painter.drawPoint(i, start.y());
		}
		return;
	}

	//transform second,thead,forth quartile line to first quartile.
	if (dx < 0 && dy > 0) {
		dx = -dx;
		quartile = 2;
	}
	else if (dx < 0 && dy < 0) {
		dx = -dx;
		dy = -dy;
		quartile = 3;
	}
	else if (dx > 0 && dy < 0) {
		dy = -dy;
		quartile = 4;
	}

	//transform line in first quartile with slope > 1 to slope < 1
	if (dx < dy) {
		slope_over_1 = true;
		myswap(dx, dy);
	}

	//draw line with Brsenham algorithm
	int points_num = dx + 1;
	painter.drawPoint(start);
	int incre_y = 0;
	int  incre_Northeast = 2 * (dy - dx);
	int  incre_East = 2 * dy;
	int P_i = 2 * dy - dx;

	for (int i = 1; i < points_num; i++) {
		if (P_i >= 0) {
			incre_y++;
			P_i += incre_Northeast;
		}
		else {
			P_i += incre_East;
		}
		
		if (slope_over_1 == false) {
			if (quartile == 1) {
				painter.drawPoint(start.x() + i,  start.y() + incre_y);
			}
			else if(quartile == 2){
				painter.drawPoint(start.x() - i,  start.y() + incre_y);
			}
			else if (quartile == 3) {
				painter.drawPoint(start.x() - i,  start.y() - incre_y);
			}
			else {
				painter.drawPoint(start.x() + i,  start.y() - incre_y);
			}
		}
		else{
			if (quartile == 1) {
				painter.drawPoint( start.x() + incre_y,  start.y() + i);
			}
			else if (quartile == 2) {
				painter.drawPoint( start.x() - incre_y,  start.y() + i);
			}
			else if (quartile == 3) {
				painter.drawPoint( start.x() - incre_y,  start.y() - i);
			}
			else {
				painter.drawPoint( start.x() + incre_y,  start.y() - i);
			}
		}
	}
}

