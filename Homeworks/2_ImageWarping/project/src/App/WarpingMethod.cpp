#include "WarpingMethod.h"

WarpingMethod::WarpingMethod() {

}

WarpingMethod::~WarpingMethod() {

}

int WarpingMethod::distance_square(QPoint point1, QPoint point2) {
	return (pow(point1.x() - point2.x(), 2) + pow(point1.y() - point2.y(), 2));
}