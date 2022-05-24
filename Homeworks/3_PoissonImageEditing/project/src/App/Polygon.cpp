#include "Polygon.h"

myPolygon::myPolygon() {

}

myPolygon::~myPolygon() {
	for (QPoint* iter : point_list_){
		delete iter;
	}
}

void myPolygon::Draw(QPainter& painter) const {
	QPoint start_= start, end_ = start;
	painter.setRenderHint(QPainter::Antialiasing);

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
	}

	painter.drawLine(end_, end);
}