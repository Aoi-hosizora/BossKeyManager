#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#ifdef WIN32  
#pragma execution_character_set("utf-8")  
#endif

#include <QtWidgets/QMainWindow>
#include "ui_MainDialog.h"

class MainDialog : public QMainWindow {
	Q_OBJECT

public:
	MainDialog(QWidget *parent = nullptr);
	~MainDialog();

private:
	Ui::MainDialogClass ui;
};

#endif // MAINDIALOG_H
