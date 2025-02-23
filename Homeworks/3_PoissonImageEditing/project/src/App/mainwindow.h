#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

class ChildWindow;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	// File IO
//	void New();
	void Open();								// Open an existing file
	void Save();								// Save image to file
	void SaveAs();
	ChildWindow *CreateChildWindow(const QString filename);
	void SetActiveSubWindow(QWidget* window);

	// Image Processing
	void Invert();								// Invert each pixel's rgb value
	void Mirror();								// Mirror image vertically or horizontally
	void GrayScale();							// Turn image to gray-scale map
	void Restore();								// Restore image to origin

	// Poisson Image Editing
	void choose_rect();							// Choose rectangle region
	void choose_polygon();						// Choose polygon region
	void clone();								// clone rect region to object image
	void mixing_clone();

private:
	void CreateActions();
	void CreateMenus();
	void CreateToolBars();
	void CreateStatusBar();

	QMdiSubWindow *FindChild(const QString &filename);
	ChildWindow* GetChildWindow();
private:							
	Ui::MainWindowClass ui;

	QMenu						*menu_file_;
	QMenu						*menu_edit_;
	QMenu						*menu_help_;
	QToolBar					*toolbar_file_;
	QToolBar					*toolbar_edit_;
//	QAction						*action_new_;
	QAction						*action_open_;
	QAction						*action_save_;
	QAction						*action_saveas_;

	QAction						*action_invert_;
	QAction						*action_mirror_;
	QAction						*action_gray_;
	QAction						*action_restore_;

	QAction						*action_choose_rect_;
	QAction						*action_choose_polygon_;
	QAction						*action_copy_;
	QAction						*action_clone_;
	QAction						*action_mixing_clone_;

	QMdiArea					*mdi_area_;
	QSignalMapper				*window_mapper_;

	ChildWindow					*child_source_;
};

#endif // MAINWINDOW_H
