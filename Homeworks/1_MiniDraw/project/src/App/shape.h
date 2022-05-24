#pragma once

#include <QtGui>

class Shape {
public:
	Shape();
	virtual ~Shape();
	virtual void Draw(QPainter& paint) = 0;
	void set_start(QPoint s);
	void set_end(QPoint e);
	void myswap(int& x, int& y);

public:
	enum Type {
		kDefault = 0,
		kLine = 1,
		kRect = 2,
		kEllipse = 3,
		kPolygon = 4,
		kFreeCurve = 5,
	};

protected:
	QPoint start;
	QPoint end;
};

