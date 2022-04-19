#include "minidraw.h"

#include <QToolBar>
#include <QIcon>
#include <QDateTime>
#include <QMessageBox>

MiniDraw::MiniDraw(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	view_widget_ = new ViewWidget();
	Creat_Action();
	Creat_ToolBar();
	Creat_Menu();
	Creat_StatusBar();
	Creat_Timer();
	setCentralWidget(view_widget_);
}

void MiniDraw::Creat_Action() {
	Action_About = new QAction(tr("&Help"), this);
	Action_About->setIcon(QIcon(":/MiniDraw/res/Help.bmp"));
	connect(Action_About, &QAction::triggered, this, &MiniDraw::AboutBox);

	Action_Line = new QAction(tr("&Line"), this);
	Action_Line->setIcon(QIcon(":/MiniDraw/res/Line.bmp"));
	connect(Action_Line, SIGNAL(triggered()), view_widget_, SLOT(setLine()));

	Action_Rect = new QAction(tr("&Rect"), this);
	Action_Rect->setIcon(QIcon(":/MiniDraw/res/Rect.bmp"));
	connect(Action_Rect, &QAction::triggered, view_widget_, &ViewWidget::setRect);

	Action_Ellipse = new QAction(tr("&Ellipse"), this);
	Action_Ellipse->setIcon(QIcon(":/MiniDraw/res/Ellipse.bmp"));
	connect(Action_Ellipse, &QAction::triggered, view_widget_, &ViewWidget::setEllipse);

	Action_Polygon = new QAction(tr("&Polygon"), this);
	Action_Polygon->setIcon(QIcon(":/MiniDraw/res/Polygon.bmp"));
	connect(Action_Polygon, &QAction::triggered, view_widget_, &ViewWidget::setPolygon);

	Action_FreeCurve = new QAction(tr("&FreeHand"), this);
	Action_FreeCurve->setIcon(QIcon(":/MiniDraw/res/FreeCurve.bmp"));
	connect(Action_FreeCurve, &QAction::triggered, view_widget_, &ViewWidget::setFreeCurve);

	Action_Del = new QAction(tr("&Undo"), this);
	Action_Del->setIcon(QIcon(":/MiniDraw/res/Delete.bmp"));
	connect(Action_Del, &QAction::triggered, view_widget_, &ViewWidget::undo);
}

void MiniDraw::Creat_ToolBar() {
	pToolBar = addToolBar(tr("&Main"));
	pToolBar->addAction(Action_About);
	pToolBar->addAction(Action_Line);
	pToolBar->addAction(Action_Rect);
	pToolBar->addAction(Action_Ellipse);
	pToolBar->addAction(Action_Polygon);
	pToolBar->addAction(Action_FreeCurve);
	pToolBar->addAction(Action_Del);
}

void MiniDraw::Creat_Menu() {
	pMenu = menuBar()->addMenu(tr("&Tools"));
	pMenu->addAction(Action_About);
	pMenu->addAction(Action_Line);
	pMenu->addAction(Action_Rect);
	pMenu->addAction(Action_Ellipse);
	pMenu->addAction(Action_Polygon);
	pMenu->addAction(Action_FreeCurve);
	pMenu->addAction(Action_Del);
}

void MiniDraw::Creat_StatusBar() {
	QPalette label_palette;
	label_palette.setColor(QPalette::Background, QColor(200, 200, 200));
	statusBar()->setAutoFillBackground(true);
	statusBar()->setPalette(label_palette);


	infoLabel = new QLabel(this);
	statusBar()->addPermanentWidget(infoLabel);

}

void MiniDraw::Creat_Timer() {
	QTimer* timer = new QTimer(this);
	timer->start(1000);

	connect(timer, &QTimer::timeout,[=](){
	infoLabel->setText(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"));
	});
}

void MiniDraw::AboutBox() {
	QMessageBox msgBox;
	msgBox.setFixedSize(200,50);
	msgBox.addButton(QMessageBox::Ok);
	msgBox.setWindowTitle(tr("Help"));
	msgBox.setText("MiniDraw V1.0");
	msgBox.exec();
	//QMessageBox::NoButton(this, tr("Help"), tr("MiniDraw V1.0"));
}

MiniDraw::~MiniDraw() {

}
