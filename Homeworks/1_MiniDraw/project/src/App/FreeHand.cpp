#include "FreeHand.h"

FreeHand::FreeHand() {

}

FreeHand::~FreeHand(){
	for (QPoint* iter : point_list_) {
		delete iter;
	}
}

void FreeHand::Draw(QPainter& painter) {
	QPoint start_ = start, end_ = start;

	if (point_list_.size() != 0) {
		for (size_t i = 0; i < point_list_.size(); i++) {
			start_ = end_;
			end_ = *point_list_[i];
			painter.drawLine(start_, end_);
		}
	}

	painter.drawLine(end_, end);
}
