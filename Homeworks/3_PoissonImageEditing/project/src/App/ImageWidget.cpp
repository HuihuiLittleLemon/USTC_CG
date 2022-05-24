#include "ImageWidget.h"
#include "Polygon.h"
#include "Rect.h"
#include "ChildWindow.h"

#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>


using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow* relatewindow) {
	draw_status_ = kNone;
	region_type_ = kRectangle;
	is_choosing_ = false;
	is_pasting_ = false;
	select_region_ = NULL;
	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);

	source_window_ = NULL;
}

ImageWidget::~ImageWidget(void) {
	if (select_region_ != NULL) {
		delete select_region_;
	}

	//if (possion_object_ != NULL) {
	//	delete possion_object_;
	//}

	//image_mat_.release();
	//image_mat_backup_.release();
}

int ImageWidget::ImageWidth() {
	return image_mat_.cols;
}

int ImageWidget::ImageHeight() {
	return image_mat_.rows;
}

void ImageWidget::set_draw_status_to_choose_rect() {
	draw_status_ = kChooseRect;	
}

void ImageWidget::set_draw_status_to_choose_polygon() {
	draw_status_ = kChoosePolygon;
}

void ImageWidget::set_draw_status_to_clone(bool mixed) {
	if (mixed == false) {
		draw_status_ = kClone;
	}
	else {
		draw_status_ = kMixingClone;
	}
}

cv::Mat ImageWidget::image() {
	return image_mat_;
}

cv::Mat ImageWidget::image_backup() {
	return image_mat_backup_;
}

void ImageWidget::set_source_window(ChildWindow* childwindow) {
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent* paintevent) {
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QImage image_show = QImage((unsigned char*)(image_mat_.data), image_mat_.cols, image_mat_.rows, image_mat_.step, QImage::Format_RGB888);
	QRect rect = QRect(0, 0, image_show.width(), image_show.height());
	painter.drawImage(rect, image_show);

	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);

	if (select_region_ != NULL) {
		select_region_->Draw(painter);
	}

	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent* mouseevent) {
	if (Qt::LeftButton == mouseevent->button() && is_choosing_ == false) {
		switch (draw_status_) {
		case kChooseRect:
			is_choosing_ = true;
			select_region_ = new Rect();
			point_start_ = point_end_ = mouseevent->pos();
			select_region_->set_start(point_start_);
			select_region_->set_end(point_end_);

			break;
		case kChoosePolygon:
			is_choosing_ = true;
			select_region_ = new myPolygon();
			this->setMouseTracking(true);						//设置鼠标释放后仍触发mouseMoveEvent
			((myPolygon*)select_region_)->drawComplete = false; //多边形绘制完成标志初始化，右键点击时置true
			point_start_ = point_end_ = mouseevent->pos();
			select_region_->set_start(point_start_);
			select_region_->set_end(point_end_);

			break;
		case kClone:
			if (is_pasting_ == false && source_window_->imagewidget_->select_region_ != NULL) {
				is_pasting_ = true;
				possion_object_ = new PossionImageEditingClass(source_window_->imagewidget_->point_start_, source_window_->imagewidget_->point_end_);
				if (source_window_->imagewidget_->region_type_ == kPolygon) {
					if (ComputationInfo::Success != possion_object_->preprocess_solver_sparse_for_polygon_region((myPolygon*)(source_window_->imagewidget_->select_region_))) {
						std::cout << "preprocess failed";
					}
				}
				else {
					if (ComputationInfo::Success != possion_object_->preprocess_solver_sparse_for_rectangle_region()) {
						std::cout << "preprocess failed";
					}
				}
			}

			update();
			break;

		case kMixingClone:
			if (is_pasting_ == false && source_window_->imagewidget_->select_region_ != NULL) {
				is_pasting_ = true;
				possion_object_ = new PossionImageEditingClass(source_window_->imagewidget_->point_start_, source_window_->imagewidget_->point_end_);
				if (source_window_->imagewidget_->region_type_ == kPolygon) {
					if (ComputationInfo::Success != possion_object_->preprocess_solver_sparse_for_polygon_region((myPolygon*)(source_window_->imagewidget_->select_region_))) {
						std::cout << "preprocess failed";
					}
				}
				else {
					if (ComputationInfo::Success != possion_object_->preprocess_solver_sparse_for_rectangle_region()) {
						std::cout << "preprocess failed";
					}
				}
			}

			update();
			break;

		default:
			break;
		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* mouseevent) {
	switch (draw_status_) {
	case kChooseRect:
	case kChoosePolygon:
		// Store point position for rectangle region
		if (is_choosing_) {
			point_end_ = mouseevent->pos();
			select_region_->set_end(point_end_);
		}
		break;

	case kClone:
		// Paste rectangle region to object image
		if (is_pasting_ && source_window_->imagewidget_->select_region_ != NULL) {
			// Restore image 
			image_mat_ = image_mat_backup_.clone();
			if (source_window_->imagewidget_->region_type_ == kPolygon) {
				possion_object_->clone_with_importing_gradients_for_polygon_region(mouseevent->pos(), source_window_->imagewidget_->image_backup(), image_mat_, false);
			}
			else {
				possion_object_->clone_with_importing_gradients_for_rectangle_region(mouseevent->pos(), source_window_->imagewidget_->image_backup(), image_mat_, false);
			}
		}
		update();
		break;

	case kMixingClone:
		if (is_pasting_ && source_window_->imagewidget_->select_region_ != NULL) {
			image_mat_ = image_mat_backup_.clone();
			if (source_window_->imagewidget_->region_type_ == kPolygon) {
				possion_object_->clone_with_importing_gradients_for_polygon_region(mouseevent->pos(), source_window_->imagewidget_->image_backup(), image_mat_, true);
			}
			else {
				possion_object_->clone_with_importing_gradients_for_rectangle_region(mouseevent->pos(), source_window_->imagewidget_->image_backup(), image_mat_, true);
			}
		}
		update();
		break;

	default:
		break;
	}

	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* mouseevent) {
	switch (draw_status_) {
	case kChooseRect:
		if (is_choosing_) {
			is_choosing_ = false;
			draw_status_ = kNone;
			region_type_ = kRectangle;
		}
		break;
	case kChoosePolygon:
		if (is_choosing_) {
			point_end_ = mouseevent->pos();
			select_region_->set_end(point_end_);
			//Polygon模式下，鼠标左键释放，记录该位置节点
			if (Qt::LeftButton == mouseevent->button())
			{
				QPoint* mid_point_ = new QPoint(mouseevent->pos());
				((myPolygon*)select_region_)->point_list_.push_back(mid_point_);
				return;
			}

			//Polygon模式下，鼠标右键释放，绘制结束
			this->setMouseTracking(false);
			((myPolygon*)select_region_)->drawComplete = true;
			region_type_ = kPolygon;
			update();
		}

		is_choosing_ = false;
		draw_status_ = kNone;
		break;

	case kClone:
		if (is_pasting_ && source_window_->imagewidget_->select_region_ != NULL) {
			image_mat_ = image_mat_backup_.clone();
			if (source_window_->imagewidget_->region_type_ == kPolygon) {
				possion_object_->clone_with_importing_gradients_for_polygon_region(mouseevent->pos(), source_window_->imagewidget_->image_backup(), image_mat_, false);
			}
			else {
				possion_object_->clone_with_importing_gradients_for_rectangle_region(mouseevent->pos(), source_window_->imagewidget_->image_backup(), image_mat_,false);
			}
			update();
			is_pasting_ = false;
			delete possion_object_;
			draw_status_ = kNone;
		}
		break;

	case kMixingClone:
		if (is_pasting_ && source_window_->imagewidget_->select_region_ != NULL) {
			image_mat_ = image_mat_backup_.clone();
			if (source_window_->imagewidget_->region_type_ == kPolygon) {
				possion_object_->clone_with_importing_gradients_for_polygon_region(mouseevent->pos(), source_window_->imagewidget_->image_backup(), image_mat_, true);
			}
			else {
				possion_object_->clone_with_importing_gradients_for_rectangle_region(mouseevent->pos(), source_window_->imagewidget_->image_backup(), image_mat_, true);

			}
			update();
			is_pasting_ = false;
			delete possion_object_;
			draw_status_ = kNone;
		}
		break;

	default:
		break;
	}

	update();
}

void ImageWidget::Open(QString filename) {
	// Load file
	if (!filename.isEmpty()) {
		image_mat_ = cv::imread(filename.toLatin1().data());
		cv::cvtColor(image_mat_, image_mat_, CV_BGR2RGB);
		image_mat_backup_ = image_mat_.clone();
	}

	cout << "image size: " << image_mat_.rows << ' ' << image_mat_.cols << endl;
	update();
}

void ImageWidget::Save() {
	SaveAs();
}

void ImageWidget::SaveAs() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull()) {
		return;
	}

	cv::Mat image_save;
	cv::cvtColor(image_mat_, image_save, cv::COLOR_BGR2RGB);
	cv::imwrite(filename.toLatin1().data(),image_save);
}

void ImageWidget::Invert() {
	if (image_mat_.empty()) {
		return;
	}

	for (cv::MatIterator_<cv::Vec3b> iter = image_mat_.begin<cv::Vec3b>(); iter != image_mat_.end<cv::Vec3b>(); iter++) {
		(*iter)[0] = 255 - (*iter)[0];
		(*iter)[1] = 255 - (*iter)[1];
		(*iter)[2] = 255 - (*iter)[2];
	}

	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical) {
	if (image_mat_.empty()) {
		return;
	}
		
	int width = image_mat_.cols;
	int height = image_mat_.rows;

	if (ishorizontal) {
		if (isvertical) {
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < height; j++) {
					image_mat_.at<cv::Vec3b>(j, i) = image_mat_backup_.at<cv::Vec3b>(height - 1 - j, width - 1 - i);
				}
			}
		} else {
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < height; j++) {
					image_mat_.at<cv::Vec3b>(j, i) = image_mat_backup_.at<cv::Vec3b>(j, width - 1 - i);
				}
			}
		}
	} else {
		if (isvertical) {
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < height; j++) {
					image_mat_.at<cv::Vec3b>(j, i) = image_mat_backup_.at<cv::Vec3b>(height - 1 - j, i);
				}
			}
		}
	}
	update();
}

void ImageWidget::TurnGray() {
	if (image_mat_.empty()) {
		return;
	}

	for (cv::MatIterator_<cv::Vec3b> iter = image_mat_.begin<cv::Vec3b>(); iter != image_mat_.end<cv::Vec3b>(); iter++) {
		int itmp = ((*iter)[0] + (*iter)[1] + (*iter)[2]) / 3;
		(*iter)[0] = itmp;
		(*iter)[1] = itmp;
		(*iter)[2] = itmp;
	}

	update();
}

void ImageWidget::Restore() {
	image_mat_ = image_mat_backup_.clone();
	point_start_ = point_end_ = QPoint(0, 0);
	if (select_region_ != NULL) {
		delete select_region_;
		select_region_ = NULL;
	}
	update();
}
