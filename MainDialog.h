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
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *);
    void on_tableWidget_currentCellChanged(int, int, int, int);

    void on_pushButton_ShowHidden_clicked();
    void on_pushButton_Refresh_clicked();
    void on_pushButton_Show_clicked();
    void on_pushButton_Setup_clicked();
    void on_pushButton_Delete_clicked();

    void on_action_Rename_triggered();
    void on_action_Pin_triggered();

    void on_comboBox_Action_currentIndexChanged(int);
    void on_checkBox_WheelAll_stateChanged(int);
    void on_checkBox_WheelSelf_stateChanged(int);

    void closeEvent(QCloseEvent *) override;

private:
    Ui::MainDialogClass ui;
    QMenu *toolButtonMenu;
    QAction *action_Show;
    QAction *action_Rename;
    QAction *action_Pin;

private:
    void initView();
    void initData();
    void refreshList();
    void setEclipseLabel(QLabel *, QString);
    void setCurrentTableWidgetRowBold(bool, bool, bool = false);
    bool setupHotKey(QKeySequence);
    void unSetupHotKey(QKeySequence);
    void unSetupAllHotKey();
    bool toContinue(Wnd *, QString, QString);
    bool toContinue(QString, QString);
};

#endif // MAINDIALOG_H
