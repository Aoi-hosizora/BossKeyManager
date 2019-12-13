#include "MainDialog.h"
#include "Utils.hpp"
#include "Global.hpp"

#include <iostream>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QKeySequenceEdit>
#include <QtWidgets/QCheckBox>
#include <QCloseEvent>
#include "qxtglobalshortcut.h"

using namespace std;

MainDialog::MainDialog(QWidget *parent) : QMainWindow(parent) {
    ui.setupUi(this);
    initView();
    initData();
}

MainDialog::~MainDialog() { }

#pragma region UILoad

// 加载界面
void MainDialog::initView() {

    // View
    setWindowFlags(Qt::Window
        | Qt::WindowCloseButtonHint
        | Qt::WindowMinimizeButtonHint);
    setFixedSize(this->width(), this->height());

    this->setWindowTitle(Global::WND_TITLE);

    // Menu
    toolButtonMenu = new QMenu(this);

    action_Show = new QAction(toolButtonMenu);
    action_Show->setText(tr("&Show"));
    action_Rename = new QAction(toolButtonMenu);
    action_Rename->setText(tr("&Rename"));
    action_Pin = new QAction(toolButtonMenu);
    action_Pin->setText(tr("&Pin"));
    action_Pin->setCheckable(true);

    connect(action_Show, SIGNAL(triggered()), this, SLOT(on_pushButton_Show_clicked()));
    connect(action_Rename, SIGNAL(triggered()), this, SLOT(on_action_Rename_triggered()));
    connect(action_Pin, SIGNAL(triggered()), this, SLOT(on_action_Pin_triggered()));

    toolButtonMenu->addAction(action_Show);
    toolButtonMenu->addAction(action_Rename);
    toolButtonMenu->addSeparator();
    toolButtonMenu->addAction(action_Pin);

    ui.pushButton_Show->setMenu(toolButtonMenu);

    // Combo
    ui.comboBox_Action->insertItems(0, QStringList() 
        << Global::NO_ACTION 
        << Global::SHOW_STATUSBAR_ICON 
        << Global::HIDE_STATUSBAR_ICON);
}

// 加载数据
void MainDialog::initData() {
    unSetupAllHotKey();
    refreshList();
}

// QLabel 超过范围显示省略号
void MainDialog::setEclipseLabel(QLabel *label, QString str) {
    QFontMetrics fontWidth(label->font());
    QString elipstr = fontWidth.elidedText(str, Qt::ElideRight, label->width());
    label->setText(elipstr);
    label->setToolTip(str);
}

#pragma endregion

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Interaction

// 列表选中改变
void MainDialog::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn) {
    if (currentRow != -1) {
        Global::CurrWnd = &Global::WindowsList.at(currentRow);
        setEclipseLabel(ui.label_Caption, Global::CurrWnd->caption);
        setEclipseLabel(ui.label_Process, QString("%1 (%2)").arg(Global::CurrWnd->image).arg(Global::CurrWnd->pid));

        if (Global::CurrWnd->actionhk == WndBKType::SHOW_STATUSBAR_ICON)
            ui.comboBox_Action->setCurrentIndex(1);
        else if (Global::CurrWnd->actionhk == WndBKType::HIDE_STATUSBAR_ICON)
            ui.comboBox_Action->setCurrentIndex(2);
        else
            ui.comboBox_Action->setCurrentIndex(0);

        ui.hotKeyEdit->setKeySequence(Global::CurrWnd->hotkey);
        ui.checkBox_ActiveToHide->setChecked(Global::CurrWnd->needActive);        
        ui.checkBox_Mute->setChecked(Global::CurrWnd->mute);

        action_Pin->setChecked(Utils::IsWindowTopMost(Global::CurrWnd->hnd));
        ui.groupBox->setEnabled(true);
        ui.pushButton_Delete->setEnabled(Global::CurrWnd->actionhk != WndBKType::NO_ACTION);
    }
}

// 选项选中修改
void MainDialog::on_comboBox_Action_currentIndexChanged(int idx) {
    ui.hotKeyEdit->setEnabled(idx != 0);
    ui.checkBox_ActiveToHide->setEnabled(idx != 0);
    ui.checkBox_Mute->setEnabled(idx != 0);
}

// 刷新
void MainDialog::on_pushButton_Refresh_clicked() {
    QString msg = QString("%1\n%2")
        .arg(tr("There are some windows hidden by the bosskey."))
        .arg(tr("Before refresh the list, you should show them."));

    if (!checkWndHideContinue("Refresh", msg)) return;
    refreshList();
}

// 显示窗口
void MainDialog::on_pushButton_Show_clicked() {
    Utils::SetActiveWindow(Global::CurrWnd->hnd);
}

// 重命名窗口
void MainDialog::on_action_Rename_triggered() {
    bool ok = false;
    QString newTitle = QInputDialog::getText(this, 
        tr("Rename caption"), 
        tr("Please input new caption: "), 
        QLineEdit::Normal, 
        Global::CurrWnd->caption, 
        &ok);
    if (ok && !newTitle.isEmpty()) {
        Utils::SetWindowName(Global::CurrWnd->hnd, newTitle);

        Global::CurrWnd->caption = newTitle;
        ui.tableWidget->item(ui.tableWidget->currentRow(), 3)->setText(Global::CurrWnd->caption);

        on_tableWidget_currentCellChanged(ui.tableWidget->currentRow(), ui.tableWidget->currentColumn(), 0, 0);
    }
}

// 置顶窗口
void MainDialog::on_action_Pin_triggered() {
    Utils::SetWindowTopMost(Global::CurrWnd->hnd, action_Pin->isChecked());

    on_tableWidget_currentCellChanged(ui.tableWidget->currentRow(), ui.tableWidget->currentColumn(), 0, 0);
}

// 设置状态
void MainDialog::on_pushButton_Setup_clicked() {
    QString msg = QString("%1\n%2")
        .arg(tr("This window is hidden by the bosskey."))
        .arg(tr("Before setup this item, you should show it."));
    if (!checkWndHideContinue(Global::CurrWnd, "Setup", msg)) return;

    unSetupHotKey(Global::CurrWnd->hotkey);

    QString combo = ui.comboBox_Action->currentText();

    if (combo != Global::SHOW_STATUSBAR_ICON && combo != Global::HIDE_STATUSBAR_ICON) {
        Global::CurrWnd->actionhk = WndBKType::NO_ACTION;
        Global::CurrWnd->hotkey = QKeySequence::NoMatch;
    }
    else {
        if (!setupHotKey(ui.hotKeyEdit->keySequence())) {
            Global::CurrWnd->actionhk = WndBKType::NO_ACTION;
            Global::CurrWnd->hotkey = QKeySequence::NoMatch;
            return;
        }
        Global::CurrWnd->hotkey = ui.hotKeyEdit->keySequence();
        if (Global::CurrWnd->hotkey == QKeySequence::NoMatch) {
            QMessageBox::critical(this, tr("Error"), tr("You must set a hotkey for this action."));
            return;
        }
        if (combo == Global::SHOW_STATUSBAR_ICON)
            Global::CurrWnd->actionhk = WndBKType::SHOW_STATUSBAR_ICON;
        else if (combo == Global::HIDE_STATUSBAR_ICON)
            Global::CurrWnd->actionhk = WndBKType::HIDE_STATUSBAR_ICON;
        Global::CurrWnd->needActive = ui.checkBox_ActiveToHide->isChecked();
        Global::CurrWnd->mute = ui.checkBox_Mute->isChecked();
    }

    QTableWidgetItem *currCell = ui.tableWidget->item(ui.tableWidget->currentRow(), 0);
    QFont b = currCell->font();
    b.setBold(true);
    currCell->setFont(b);

    on_tableWidget_currentCellChanged(ui.tableWidget->currentRow(), ui.tableWidget->currentColumn(), 0, 0);
}

// 删除状态
void MainDialog::on_pushButton_Delete_clicked() {
    // 判断窗口是否隐藏
    if (Global::CurrWnd->actionhk == WndBKType::HIDE_STATUSBAR_ICON) {
        QString msg = QString("%1\n%2")
            .arg(tr("This window is hidden by the bosskey."))
            .arg(tr("Before delete the setting, you should show it."));
        if (!checkWndHideContinue(Global::CurrWnd, "Delete", msg))
            return;
    }
    Global::CurrWnd->actionhk = WndBKType::NO_ACTION;
    ui.comboBox_Action->setCurrentIndex(0);

    unSetupHotKey(Global::CurrWnd->hotkey);
    Global::CurrWnd->hotkey = QKeySequence::NoMatch;
    ui.hotKeyEdit->setKeySequence(QKeySequence::NoMatch);
    Global::CurrWnd->needActive = false;
    Global::CurrWnd->mute = false;

    QTableWidgetItem *currCell = ui.tableWidget->item(ui.tableWidget->currentRow(), 0);
    QFont b = currCell->font();
    b.setBold(false);
    currCell->setFont(b);
    
    on_tableWidget_currentCellChanged(ui.tableWidget->currentRow(), ui.tableWidget->currentColumn(), 0, 0);
}

// 显示所有隐藏
void MainDialog::on_pushButton_ShowAllWindowsHidden_clicked() {
    int cnt = 0;
    foreach (Wnd wnd, Global::WindowsList) {
        if (wnd.actionhk == WndBKType::HIDE_STATUSBAR_ICON && IsWindow(wnd.hnd) && !IsWindowVisible(wnd.hnd)) {
            cnt++;
            Utils::UnHideWindow(wnd.hnd);
            Utils::SetMute(&wnd, false);
        }
    }
    if (cnt == 0)
        QMessageBox::information(this, "Show", 
        "There is no window hidden.");
    else
        QMessageBox::information(this, "Show", 
        (cnt == 1) ? "1 window shown finished" : QString("%1%2").arg(cnt).arg(" windows shown finished"));
}

// 窗口关闭
void MainDialog::closeEvent(QCloseEvent *e) {
    // 判断是否有被隐藏的窗口
    QString msg = QString("%1\n%2")
        .arg(tr("There are some windows hidden by the bosskey."))
        .arg(tr("Before close the program, you should show them."));
    if (!checkWndHideContinue("Exit", msg))
        e->ignore();
    else
        e->accept();
}

#pragma endregion

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Functions

// 加载列表内容
void MainDialog::refreshList() {
    int allRows = ui.tableWidget->rowCount();
    for (int i = 0; i < allRows; i++)
        ui.tableWidget->removeRow(0);

    ui.tableWidget->setHorizontalHeaderLabels(
        QStringList() << tr("Handle") << tr("Pid") << tr("Process") << tr("Caption")  
    );
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui.tableWidget->verticalHeader()->setDefaultSectionSize(20);

    ui.label_Caption->setText(tr("Not selected"));
    ui.label_Process->setText(tr("Not selected"));
    ui.hotKeyEdit->setKeySequence(QKeySequence::NoMatch);
    ui.groupBox->setEnabled(false);
    ui.comboBox_Action->setCurrentIndex(0);

    vector<Wnd> allWnds = Utils::getAllWindows();
    foreach (Wnd wnd, allWnds) {
        int row = ui.tableWidget->rowCount();
        ui.tableWidget->insertRow(row);
        ui.tableWidget->setItem(row, 0, new QTableWidgetItem(wnd.hndString()));
        ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(wnd.pid)));
        ui.tableWidget->setItem(row, 2, new QTableWidgetItem(wnd.image));
        ui.tableWidget->setItem(row, 3, new QTableWidgetItem(wnd.caption));
    }
    ui.label_Title->setText(QString(tr("A&ctive Windows: (All %1)")).arg(allWnds.size()));

    Global::WindowsList = allWnds;
    ui.groupBox->setEnabled(false);
}

// 注册热键
bool MainDialog::setupHotKey(QKeySequence key) {
    foreach (QxtGlobalShortcut *sc, Global::shortcuts) {
        if (sc->shortcut() == key)
            return true;
    }

    auto shortcut = new QxtGlobalShortcut(this);
    if (!shortcut->setShortcut(key)) {
        QMessageBox::critical(this,
            "Error",
            "This hotkey " + key.toString() + " has been used.");
        return false;
    } 
    else {
        connect(shortcut, &QxtGlobalShortcut::activated, [=]() {

            foreach (Wnd wnd, Global::WindowsList) {
                if (wnd.actionhk != WndBKType::NO_ACTION && wnd.hotkey == key) {
                    // 状态栏显示 ----------------- 1
                    if (wnd.actionhk == WndBKType::SHOW_STATUSBAR_ICON) {
                        // 需要活动窗口去显示和隐藏
                        if (wnd.needActive && wnd.hnd != GetForegroundWindow()) 
                            return;

                        // 当前是否显示
                        bool isShow = !IsIconic(wnd.hnd);

                        // 显示：静音，最小化
                        // 最小化：取消静音，还原
                        if (wnd.mute) 
                            Utils::SetMute(&wnd, isShow);

                        if (isShow)
                            Utils::MinimizeWindow(wnd.hnd);
                        else
                            Utils::RestoreWindow(wnd.hnd);
                    }
                    // 状态栏不显示 ----------------- 2
                    else {
                        // 当前是否显示
                        bool isShow = IsWindowVisible(wnd.hnd);

                        // 显示：静音，隐藏
                        // 隐藏：取消静音，显示
                        if (wnd.mute) 
                            Utils::SetMute(&wnd, isShow);

                        if (isShow) {
                            // 隐藏需要活动窗口
                            if (wnd.needActive && wnd.hnd != GetForegroundWindow()) 
                                return;
                            Utils::HideWindow(wnd.hnd);
                        }
                        else
                            // 显示不考虑活动窗口
                            Utils::UnHideWindow(wnd.hnd);
                    }
                }
            }
        });
        Global::shortcuts.push_back(shortcut);
        return true;
    }
}

// 注销热键
void MainDialog::unSetupHotKey(QKeySequence key) {
    for (auto iter = Global::shortcuts.begin(); iter != Global::shortcuts.end();) {
        QxtGlobalShortcut *sc = *iter;
        if (sc->shortcut() == key) {
            disconnect(sc);
            Global::shortcuts.erase(iter);
            delete sc;
        }
        else
            iter++;
    }
}

// 注销所有热键
void MainDialog::unSetupAllHotKey() {
    for (auto iter = Global::shortcuts.begin(); iter != Global::shortcuts.end();) {
        QxtGlobalShortcut *sc = *iter;
        disconnect(sc);
        Global::shortcuts.erase(iter);
        delete sc;
    }
}

// 检查所有窗口是否被隐藏
// 并且判断是否需要注销以及显示
bool MainDialog::checkWndHideContinue(QString title, QString msg) {
    bool has = false;
    foreach (Wnd wnd, Global::WindowsList) {
        if (wnd.actionhk ==  WndBKType::HIDE_STATUSBAR_ICON && // 窗口操作为隐藏图标
            IsWindow(wnd.hnd) && !IsWindowVisible(wnd.hnd)) // 该窗口还存在并且被隐藏
            has = true;
    }

    // 不存在隐藏窗口，继续操作
    if (!has) {
        unSetupAllHotKey();
        return true;
    }

    // 存在隐藏窗口，判断
    QMessageBox::StandardButton result = QMessageBox::information(this,
        title, msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    // 不操作
    if (result == QMessageBox::No) 
        return false;
    
    // 刷新后操作
    unSetupAllHotKey();
    on_pushButton_ShowAllWindowsHidden_clicked();
    return true;
}

// 检查指定是否被隐藏
bool MainDialog::checkWndHideContinue(Wnd *wnd, QString title, QString msg) {
    if (IsWindow(wnd->hnd) && !IsWindowVisible(wnd->hnd)) {
        // 被隐藏，判断
        QMessageBox::StandardButton result = QMessageBox::information(this, 
            title, msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        // 不操作
        if (result == QMessageBox::No) 
            return false;

        // 显示后操作
        Utils::UnHideWindow(wnd->hnd);
        Utils::SetMute(wnd, false);
        return true;
    }
    // 不被隐藏
    return true;
}

#pragma endregion