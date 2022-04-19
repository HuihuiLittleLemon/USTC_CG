#include "Polygon.h"

myPolygon::myPolygon() {

}

myPolygon::~myPolygon() {
	for (QPoint* iter : ponit_list_){
		delete iter;
	}
}

void myPolygon::Draw(QPainter& painter) const {
	QPoint start_= start, end_ = start;
	painter.setRenderHint(QPainter::Antialiasing);

	if (ponit_list_.size() != 0) {		
		for (size_t i = 0; i < ponit_list_.size(); i++) {
			start_ = end_;
			end_ = *ponit_list_[i];
			painter.drawLine(start_, end_);
		}
	}
	if (this->drawComplete == true)
	{
		painter.drawLine(start, end);
	}

	painter.drawLine(end_, end);
}