#pragma once

#include "IDW.h"

#include <QWidget>
#include <Qevent>
#include <QImage>
#include <QPair>


QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

class ImageWidget : public QWidget {
	Q_OBJECT

public:
	ImageWidget(void);
	~ImageWidget(void);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void set_seting_anchor_points_flag();						// Set anchor points flag
protected:
	void paintEvent(QPaintEvent *paintevent);

public slots:
	// File IO
	void Open();												// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal=false, bool vertical=true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin

	void warping_with_IDW();
	void warping_with_RBF();
	void warping(WarpingMethod &warp);
private:
	QPoint covert_local_point_to_global(QPoint point);
	void draw_line_between_anchor_couples(QPainter &painter);
	void ImageWidget::draw_arrow_line(QPoint start, QPoint end, QPainter &painter);

public:
	QPoint* start;
	QPoint* end;
	QPoint picOrig;
private:
	QImage		*ptr_image_;									// image 
	QImage		*ptr_image_backup_;
	std::vector<QPoint> points_p_;							
	std::vector<QPoint> points_q_;

	bool image_loaded_;	 
	bool seting_anchor_points_;
	QPoint current_mouse_position_;								//store current mouse position when seting anchor points
};
