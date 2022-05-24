#include "viewwidget.h"
#include <QDebug>

ViewWidget::ViewWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	draw_status_ = false;
	shape_ = NULL;
	type_ = Shape::kDefault;
}

ViewWidget::~ViewWidget()
{
	for (size_t i = 0; i < shape_list_.size(); i++)
	{
		if (shape_list_[i])
		{
			delete shape_list_[i];
			shape_list_[i] = NULL;
		}
	}
	shape_list_.clear();
}

void ViewWidget::setLine()
{
	type_ = Shape::kLine;
}

void ViewWidget::setRect()
{
	type_ = Shape::kRect;
}

void ViewWidget::setEllipse()
{
	type_ = Shape::kEllipse;
}

void ViewWidget::setPolygon()
{
	type_ = Shape::kPolygon;
}

void ViewWidget::setFreeCurve()
{
	type_ = Shape::kFreeCurve;
}

void ViewWidget::undo()
{
	if (!shape_list_.empty())
	{
		delete shape_list_.back();
		shape_list_.pop_back();
		update();
	}
}

void ViewWidget::mousePressEvent(QMouseEvent* event)
{
	if (Qt::LeftButton == event->button()&& draw_status_ == false)
	{
		switch (type_)
		{
		case Shape::kLine:
			shape_ = new Line();
			break;
		case Shape::kRect:
			shape_ = new Rect();
			break;
		case Shape::kEllipse:
			shape_ = new myEllipse();
			break;
		case Shape::kPolygon:
			shape_ = new myPolygon();
			this->setMouseTracking(true);				//��������ͷź��Դ���mouseMoveEvent
			((myPolygon*)shape_)->drawComplete = false; //����λ�����ɱ�־��ʼ�����Ҽ����ʱ��true
			break;
		case Shape::kFreeCurve:
			shape_ = new FreeHand();	
			break;
		case Shape::kDefault:
			break;
		}
		if (shape_ != NULL)
		{
			draw_status_ = true;
			start_point_ = end_point_ = event->pos();
			shape_->set_start(start_point_);
			shape_->set_end(end_point_);
		}
			
	}

	update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (draw_status_ && shape_ != NULL)
	{
		end_point_ = event->pos();
		shape_->set_end(end_point_);

		//FreeHandģʽ�£�ÿ������ƶ�����¼��굱ǰλ��
		if (type_ == Shape::kFreeCurve)
		{
			QPoint* mid_point_ = new QPoint(end_point_);
			((FreeHand*)shape_)->point_list_.push_back(mid_point_);
		}
		update();
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (shape_ != NULL)
	{
		if (type_ == Shape::kPolygon)
		{	//Polygonģʽ�£��������ͷţ���¼��λ�ýڵ�
			if (Qt::LeftButton == event->button())
			{
				QPoint* mid_point_ = new QPoint(event->pos());
				((myPolygon*)shape_)->point_list_.push_back(mid_point_);
				return;
			}

			//Polygonģʽ�£�����Ҽ��ͷţ����ƽ���
			this->setMouseTracking(false);
			((myPolygon*)shape_)->drawComplete = true;
			update();
		}
		
		draw_status_ = false;
		shape_list_.push_back(shape_);
		shape_ = NULL;
	}
}

void ViewWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
		
	for (int i = 0; i < shape_list_.size(); i++)
	{
		shape_list_[i]->Draw(painter);
	}

	if (shape_ != NULL) {
		shape_->Draw(painter);
	}

}