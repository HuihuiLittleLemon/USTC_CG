#include "FreeCurve.h"

FreeCurve::FreeCurve() {

}

FreeCurve::~FreeCurve(){
	for (QPoint* iter : ponit_list_) {
		delete iter;
	}
}

void FreeCurve::Draw(QPainter& painter)const {
	QPoint start_ = start, end_ = start;

	if (ponit_list_.size() != 0) {
		for (size_t i = 0; i < ponit_list_.size(); i++) {
			start_ = end_;
			end_ = *ponit_list_[i];
			painter.drawLine(start_, end_);
		}
	}

	painter.drawLine(end_, end);
}
