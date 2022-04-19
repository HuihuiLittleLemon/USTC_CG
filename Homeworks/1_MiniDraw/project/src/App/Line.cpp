#include "Line.h"

Line::Line() {

}

Line::~Line() {

}

void Line::Draw(QPainter& painter)const {
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawLine(start, end);
}
