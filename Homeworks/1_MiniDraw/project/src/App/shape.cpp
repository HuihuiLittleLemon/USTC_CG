#include "Shape.h"

Shape::Shape() {

}

Shape::~Shape() {

}

void Shape::set_start(QPoint s) {
	start = s;
}

void Shape::set_end(QPoint e) {
	end = e;
}

void Shape::myswap(int& x, int& y) {
	int tmp = x;
	x = y;
	y = tmp;
}