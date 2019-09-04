#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#ifdef WIN32  
#pragma execution_character_set("utf-8")  
#endif

#include "ui_MainDialog.h"
#include <QtWidgets/QMainWindow>

class MainDialog : public QMainWindow {
	Q_OBJECT

public:
	MainDialog(QWidget *parent = nullptr);
	~MainDialog();

private slots:
	void on_listWidget_currentRowChanged(int);
	void on_pushButton_Refresh_clicked();
	void on_pushButton_Show_clicked();
	void on_pushButton_Rename_clicked();
	void on_pushButton_Setup_clicked();
	void on_pushButton_Delete_clicked();

private:
	Ui::MainDialogClass ui;
	void onLoaded();
	void loadWindowsList();
	void setEclipseLabel(QLabel *, QString, int);
	bool setupHotKey(QKeySequence);
	void unSetupHotKey(QKeySequence);

};

#endif // MAINDIALOG_H
