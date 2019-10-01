#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#ifdef WIN32  
#pragma execution_character_set("utf-8")
#endif

#include "ui_MainDialog.h"
#include "Wnd.h"
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
	void on_action_Rename_triggered();
	void on_action_Pin_triggered();

	void on_pushButton_Setup_clicked();
	void on_pushButton_Delete_clicked();

	void on_pushButton_ShowAllWindowsHidden_clicked();
	void on_comboBox_Action_currentIndexChanged(int);

	void closeEvent(QCloseEvent *) override;

private:
	Ui::MainDialogClass ui;
	QMenu *toolButtonMenu;
	QAction *action_Show;
	QAction *action_Rename;
	QAction *action_Pin;

private:
	void onLoaded();
	void initMenuAction();
	void loadWindowsList();
	void setEclipseLabel(QLabel *, QString);
	bool setupHotKey(QKeySequence);
	void unSetupHotKey(QKeySequence);
	void unSetupAllHotKey();
	bool checkWndHideContinue(Wnd *, QString, QString);
	bool checkWndHideContinue(QString, QString);

};

#endif // MAINDIALOG_H
