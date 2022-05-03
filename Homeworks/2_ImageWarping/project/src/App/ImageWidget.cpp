#include "ImageWidget.h"
#include "IDW.h"
#include "RBF.h"

#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <QPoint>
#include <QPen>
#include <QColor>
#include <iostream>

using std::cout;
using std::endl;

ImageWidget::ImageWidget(void)
{
	ptr_image_ = new QImage();
	ptr_image_backup_ = new QImage();
	image_loaded_ = false;
	seting_anchor_points_ = false;
}


ImageWidget::~ImageWidget(void){
	points_p_.clear();
	points_q_.clear();
	delete ptr_image_;
	delete ptr_image_backup_;
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect((width() - ptr_image_->width()) / 2, (height() - ptr_image_->height()) / 2, ptr_image_->width(), ptr_image_->height());
	painter.drawImage(rect, *ptr_image_);

	// Draw anchor points
	draw_line_between_anchor_couples(painter);

	painter.end();
}
void ImageWidget::draw_line_between_anchor_couples(QPainter &painter) {
	if (seting_anchor_points_ == false || points_p_.size() == 0) {
		return;
	}

	size_t i = 0;
	QPen pen;
	pen.setColor(Qt::red);
	pen.setWidthF(3.5);
	painter.setPen(pen);
	
	//if anchor point p equal anchor point q,then just draw point p,otherwise draw arrow line between  p and q.
	for (; i < points_p_.size()-1; i++) {
		if (points_p_[i] == points_q_[i]) {
			painter.drawPoint(covert_local_point_to_global(points_p_[i]));
		} else {
			draw_arrow_line(covert_local_point_to_global(points_p_[i]), covert_local_point_to_global(points_q_[i]), painter);
		}
	}

	if (points_p_.size() == points_q_.size()) {
		if (points_p_[i] == points_q_[i]) {
			painter.drawPoint(covert_local_point_to_global(points_p_[i]));
		}
		else {
			draw_arrow_line(covert_local_point_to_global(points_p_[i]), covert_local_point_to_global(points_q_[i]), painter);
		}
	}
	else {
		if (covert_local_point_to_global(points_p_[i]) == current_mouse_position_) {
			painter.drawPoint(current_mouse_position_);
		}
		else {
			draw_arrow_line(covert_local_point_to_global(points_p_[i]), current_mouse_position_, painter);
		}
	}
}

void ImageWidget::draw_arrow_line(QPoint start, QPoint end, QPainter &painter) {

	const float PI = 3.14159f;
	float line_len = sqrt(pow(start.x() - end.x(), 2) + pow(start.y() - end.y(), 2));
	float arrow_len = 5;
	float angle = 45;
	
	float unit_vector_x = (float)(start.x() - end.x()) / line_len;
	float unit_vector_y = (float)(start.y() - end.y()) / line_len;

	float arrow1_end_x = end.x() + arrow_len * (cos(PI * angle / 180) * unit_vector_x + sin(PI * angle / 180) * unit_vector_y);
	float arrow1_end_y = end.y() + arrow_len * (-sin(PI * angle / 180) * unit_vector_x + cos(PI * angle / 180) * unit_vector_y);
	float arrow2_end_x = end.x() + arrow_len * (cos(PI * angle / 180) * unit_vector_x - sin(PI * angle / 180) * unit_vector_y);
	float arrow2_end_y = end.y() + arrow_len * (sin(PI * angle / 180) * unit_vector_x + cos(PI * angle / 180) * unit_vector_y);

	QPoint arrow1_start(round(arrow1_end_x + 0.5), round(arrow1_end_y + 0.5));
	QPoint arrow2_start(round(arrow2_end_x + 0.5), round(arrow2_end_y + 0.5));
	painter.drawLine(start,end);
	painter.drawLine(arrow1_start, end);
	painter.drawLine(arrow2_start, end);
}

QPoint ImageWidget::covert_local_point_to_global(QPoint point) {
	return QPoint(point.x() + picOrig.x(), point.y() + picOrig.y());
}

void ImageWidget::Open()
{
	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), "../data", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		ptr_image_->load(fileName);
		*(ptr_image_backup_) = *(ptr_image_);
	}
	picOrig.setX((width() - ptr_image_->width()) / 2);
	picOrig.setY((height() - ptr_image_->height()) / 2);
	image_loaded_ = true;
	//ptr_image_->invertPixels(QImage::InvertRgb);
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	//*(ptr_image_) = ptr_image_->rgbSwapped();
	cout << "image size: " << ptr_image_->width() << ' ' << ptr_image_->height() << endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	ptr_image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i = 0; i < ptr_image_->width(); i++)
	{
		for (int j = 0; j < ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			ptr_image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}

	// equivalent member function of class QImage
	// ptr_image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical) {
	QImage image_tmp(*(ptr_image_));
	int width = ptr_image_->width();
	int height = ptr_image_->height();

	if (ishorizontal) {
		if (isvertical) {
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < height; j++) {
					ptr_image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, height - 1 - j));
				}
			}
		} else {			//仅水平翻转
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < height; j++) {
					ptr_image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
				}
			}
		}
	} else {
		if (isvertical) {   //仅垂直翻转
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < height; j++) {
					ptr_image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	// *(ptr_image_) = ptr_image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray() {
	for (int i = 0; i < ptr_image_->width(); i++) {
		for (int j = 0; j < ptr_image_->height(); j++) {
			QRgb color = ptr_image_->pixel(i, j);
			//Gray = (Red * 0.3 + Green * 0.59 + Blue * 0.11)
			int gray_value = qRed(color) * 0.3 + qGreen(color) * 0.59 + qBlue(color) * 0.11;

			//Gray = (Red + Green + Blue) / 3
			//int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
			ptr_image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
		}
	}

	update();
}

void ImageWidget::Restore() {
	points_p_.clear();
	points_q_.clear();
	*(ptr_image_) = *(ptr_image_backup_);
	update();
}

void ImageWidget::set_seting_anchor_points_flag() {
	if (image_loaded_ == false) {
		return;
	}
	this->setMouseTracking(true);
	seting_anchor_points_= true;
	points_p_.clear();
	points_q_.clear();
}

void ImageWidget::warping_with_RBF() {
	RBF rbf(points_p_, points_q_, 50, -0.5);
	warping(rbf);
}

void  ImageWidget::warping_with_IDW() {
	IDW idw(points_p_, points_q_, false);
	//IDW idw(points_p_, points_q_);
	warping(idw);
}

void ImageWidget::warping(WarpingMethod &warp){
	if (points_p_.size() == 0) {
		return;
	}
	seting_anchor_points_ = false;
	int width = ptr_image_->width();
	int height = ptr_image_->height();
	QImage image_tmp = *ptr_image_;                       
	QPoint result(0, 0);
	int valid_pixel_number;
	QColor pixel_color;
	int red, green, blue;
	bool** point_filled = new bool* [width];	// indicate whether pixel had filled during wraping
	for (int i = 0; i < width; i++) {
		point_filled[i] = new bool[height];
		for (int j = 0; j < height; j++) {
			point_filled[i][j] = false;
		}
	}

	ptr_image_->fill(Qt::white);
	for (int i = 0; i < width; i++)
	 {
		for (int j = 0; j < height; j++)
		{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
			result = warp.f_func(QPoint(i, j));
			if (result.x() < 0 || result.y() < 0 || result.x() >= width || result.y() >= height) {
				continue;
			}
			ptr_image_->setPixel(result.x(), result.y(), image_tmp.pixel(i, j));
			point_filled[result.x()][result.y()] = true;
		}
	}

	//eliminate white seams,filled with the average of the surrounding 8 pixels
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			valid_pixel_number = 0;
			red = 0; 
			green = 0;
			blue = 0;
			if (point_filled[i][j] == false) {
				if ((i - 1) < width && (j - 1) < height &&(i - 1) >= 0 && (j - 1) >= 0 && point_filled[i-1][j-1] == true) {
					valid_pixel_number++;
					pixel_color = QColor(ptr_image_->pixel(i - 1, j - 1));
					red += pixel_color.red();
					blue += pixel_color.blue();
					green+= pixel_color.green();
				}
				if (i < width && (j - 1) < height && i >= 0 && (j - 1) >= 0 && point_filled[i][j-1] == true) {
					valid_pixel_number++;
					pixel_color = QColor(ptr_image_->pixel(i, j - 1));
					red += pixel_color.red();
					blue += pixel_color.blue();
					green += pixel_color.green();
				}
				if ((i + 1) < width && (j - 1) < height && (i + 1) >= 0 && (j - 1) >= 0 && point_filled[i+1][j-1] == true) {
					valid_pixel_number++;
					pixel_color = QColor(ptr_image_->pixel(i + 1, j - 1));
					red += pixel_color.red();
					blue += pixel_color.blue();
					green += pixel_color.green();
				}

				if ((i - 1) < width && j < height && (i - 1) >= 0 && j >= 0 && point_filled[i-1][j] == true) {
					valid_pixel_number++;
					pixel_color = QColor(ptr_image_->pixel(i - 1, j));
					red += pixel_color.red();
					blue += pixel_color.blue();
					green += pixel_color.green();
				}
				if ((i + 1) < width && j < height && (i + 1) >= 0 && j >= 0 && point_filled[i+1][j] == true) {
					valid_pixel_number++;
					pixel_color = QColor(ptr_image_->pixel(i + 1, j));
					red += pixel_color.red();
					blue += pixel_color.blue();
					green += pixel_color.green();
				}

				if ((i - 1) < width && (j + 1) < height && (i - 1) >= 0 && (j + 1) >= 0 && point_filled[i-1][j+1] == true) {
					valid_pixel_number++;
					pixel_color = QColor(ptr_image_->pixel(i - 1, j + 1));
					red += pixel_color.red();
					blue += pixel_color.blue();
					green += pixel_color.green();
				}
				if (i < width && (j + 1) < height && i >= 0 && (j + 1) >= 0 && point_filled[i][j+1] == true) {
					valid_pixel_number++;
					pixel_color = QColor(ptr_image_->pixel(i, j + 1));
					red += pixel_color.red();
					blue += pixel_color.blue();
					green += pixel_color.green();
				}
				if ((i + 1) < width && (j + 1) < height && (i + 1) >= 0 && (j + 1) >= 0 && point_filled[i+1][j+1] == true) {
					valid_pixel_number++;
					pixel_color = QColor(ptr_image_->pixel(i + 1, j + 1));
					red += pixel_color.red();
					blue += pixel_color.blue();
					green += pixel_color.green();
				}

				if (valid_pixel_number != 0) {
					red /= valid_pixel_number;
					blue /= valid_pixel_number;
					green /= valid_pixel_number;
					ptr_image_->setPixel(i,j, qRgb(red,green,blue));
					//point_filled[i][j] = true;
				}
			}
		}
	}

	for (int i = 0; i < width; i++) {
		delete point_filled[i];
	}
	delete []point_filled;

	update();
}

void ImageWidget::mousePressEvent(QMouseEvent* event) {
	if (seting_anchor_points_ == false) {
		return;
	}

	if (Qt::LeftButton == event->button())
	{
		
		QPoint start(event->pos().x()- picOrig.x(), event->pos().y() - picOrig.y()); //calculate mouse position on ImageWidget
		points_p_.push_back(start);

		current_mouse_position_.setX(event->pos().x());
		current_mouse_position_.setY(event->pos().y());	
	} else if (Qt::RightButton == event->button() && !points_p_.empty()) {			//click rightbutton to delete lastest anchor points input
		points_p_.pop_back();
		points_q_.pop_back();
	}	

	update();
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event) {

	if ((Qt::LeftButton & event->buttons()) && seting_anchor_points_ == true)
	{
		current_mouse_position_.setX(event->pos().x());
		current_mouse_position_.setY(event->pos().y());
		update();
	}
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (Qt::LeftButton == event->button())
	{
		QPoint end(event->pos().x() - picOrig.x(), event->pos().y() - picOrig.y());
		if (seting_anchor_points_ == true) {
			points_q_.push_back(end);
			update();
		}
	}
}
