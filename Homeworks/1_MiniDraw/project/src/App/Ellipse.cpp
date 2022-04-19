#include "Ellipse.h"

myEllipse::myEllipse() {

}

myEllipse::~myEllipse() {

}

void myEllipse::Draw(QPainter& painter)const {
	painter.drawEllipse(start.x(), start.y(), end.x() - start.x(), end.y() - start.y());
}