#pragma once

#include <ui_minidraw.h>
#include <viewwidget.h>

#include <QtWidgets/QMainWindow>
#include <qmessagebox.h>
#include <QLabel>
#include <QTimer>

class MiniDraw : public QMainWindow {
	Q_OBJECT

public:
	MiniDraw(QWidget* parent = 0);
	~MiniDraw();

	QMenu* pMenu;
	QToolBar* pToolBar;
	QAction* Action_About;
	QAction* Action_Line;
	QAction* Action_Rect;
	QAction* Action_Ellipse;
	QAction* Action_Polygon;
	QAction* Action_FreeHand;

	QLabel* infoLabel;
	QAction* Action_Del;
	QAction* Action_Fill;
	QTimer* timer;

	void Creat_Menu();
	void Creat_ToolBar();
	void Creat_Action();
	void Creat_StatusBar();
	void Creat_Timer();

private slots:void AboutBox();

private:
	Ui::MiniDrawClass ui;
	ViewWidget* view_widget_;
};
