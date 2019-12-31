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

#pragma region LoadUI

// 加载界面
void MainDialog::initView() {

    // View
    setWindowFlags(Qt::Window
        | Qt::WindowCloseButtonHint
        | Qt::WindowMinimizeButtonHint);
    // setFixedSize(this->width(), this->height());
    this->setMinimumSize(this->sizeHint());
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

// 设置 TableWidget 当前行粗体
void MainDialog::setCurrentTableWidgetRowBold(bool isBold, bool isKOrW, bool isAll) {
    if (ui.tableWidget->currentRow() == -1 && !isAll)
        return;

    int f = ui.tableWidget->currentRow(), t = f + 1;
    if (isAll) {
        f = 0;
        t = ui.tableWidget->rowCount();
    }
    for (int r = f; r < t; r++) {
        ui.tableWidget->item(r, isKOrW ? 1 : 2)->setText(isBold ? "*" : "");
        bool isb = ui.tableWidget->item(r, 1)->text() == "*" || ui.tableWidget->item(r, 2)->text() == "*";

        for (int c = 0; c < ui.tableWidget->horizontalHeader()->count(); c++) {
            QTableWidgetItem *currCell = ui.tableWidget->item(r, c);
            QFont b = currCell->font();
            b.setBold(isb);
            currCell->setFont(b);
        }
    }
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
        setEclipseLabel(ui.label_Process, tr("%1 (%2)").arg(Global::CurrWnd->image).arg(Global::CurrWnd->pid));

        ui.hotKeyEdit->setKeySequence(Global::CurrWnd->setting.hotkey);
        if (Global::CurrWnd->setting.wndAction == WndBKType::SHOW_STATUSBAR_ICON)
            ui.comboBox_Action->setCurrentIndex(1);
        else if (Global::CurrWnd->setting.wndAction == WndBKType::HIDE_STATUSBAR_ICON)
            ui.comboBox_Action->setCurrentIndex(2);
        else
            ui.comboBox_Action->setCurrentIndex(0);

        ui.checkBox_NeedActive->setChecked(Global::CurrWnd->setting.needActive);
        ui.checkBox_NeedMute->setChecked(Global::CurrWnd->setting.needMute);

        action_Pin->setChecked(Utils::IsWindowTopMost(Global::CurrWnd->hnd));
        ui.groupBox_Key->setEnabled(true);
        ui.pushButton_Delete->setEnabled(Global::CurrWnd->setting.wndAction != WndBKType::NO_ACTION);

        if (!Global::isGlobalWheel) {
            ui.checkBox_WheelSelf->setChecked(Global::CurrWnd->setting.wheelFunction);
        }
    } else {
        ui.label_Caption->setText(tr("Not selected"));
        ui.label_Process->setText(tr("Not selected"));

        ui.hotKeyEdit->setKeySequence(QKeySequence::NoMatch);
        ui.comboBox_Action->setCurrentIndex(0);

        ui.checkBox_NeedActive->setChecked(false);
        ui.checkBox_NeedMute->setChecked(false);

        action_Pin->setChecked(false);
        ui.groupBox_Key->setEnabled(false);
        ui.pushButton_Delete->setEnabled(false);

        ui.checkBox_WheelSelf->setEnabled(false);
    }
}

// 选项选中修改
void MainDialog::on_comboBox_Action_currentIndexChanged(int idx) {
    ui.hotKeyEdit->setEnabled(idx != 0);
    ui.checkBox_NeedActive->setEnabled(idx != 0);
    ui.checkBox_NeedMute->setEnabled(idx != 0);
}

// 双击弹出菜单
void MainDialog::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item) {
    ui.pushButton_Show->showMenu();
}

// 所有窗口 键盘上下键换成鼠标滚轮
// https://libqxt.bitbucket.io/doc/tip/index.html
// https://libqxt.bitbucket.io/doc/tip/qxtglobalshortcut.html
// QtxBug: K+ W+ K- W(x)
//         W+ K- W- K(x)
//         K+ W+ W- K-
//         W+ K+ K- W-
void MainDialog::on_checkBox_WheelAll_stateChanged(int state) {
    if (state == Qt::Checked) {
        if (Global::wheelDownKeys != nullptr) {
            Global::wheelDownKeys->disconnect();
            delete Global::wheelDownKeys;
            Global::wheelDownKeys = nullptr;
        }
        if (Global::wheelUpKeys != nullptr) {
            Global::wheelUpKeys->disconnect();
            delete Global::wheelUpKeys;
            Global::wheelUpKeys = nullptr;
        }
        Global::wheelDownKeys = new QxtGlobalShortcut(this);
        Global::wheelUpKeys = new QxtGlobalShortcut(this);
        if (!Global::wheelDownKeys->setShortcut(QKeySequence(QObject::tr("down")))
            || !Global::wheelUpKeys->setShortcut(QKeySequence(QObject::tr("up")))) {
                delete Global::wheelDownKeys;
                delete Global::wheelUpKeys;
                Global::wheelDownKeys = Global::wheelUpKeys = nullptr;
                QMessageBox::critical(this, tr("Wheel function"), tr("The up/down key has already been used, this function is unable"));
                ui.checkBox_WheelAll->setChecked(false);
                return;
        }

        connect(Global::wheelUpKeys, &QxtGlobalShortcut::activated, [=]() {
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHEEL_DELTA, 0);
        });
        connect(Global::wheelDownKeys, &QxtGlobalShortcut::activated, [=]() {
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -WHEEL_DELTA, 0);
        });

        Global::isGlobalWheel = true;
        ui.checkBox_WheelSelf->setEnabled(false);
        ui.checkBox_WheelSelf->setChecked(true);

        setCurrentTableWidgetRowBold(true, false, true);
    }
    else {
        if (Global::wheelDownKeys != nullptr) {
            Global::wheelDownKeys->disconnect();
            delete Global::wheelDownKeys;
            Global::wheelDownKeys = nullptr;
        }
        if (Global::wheelUpKeys != nullptr) {
            Global::wheelUpKeys->disconnect();
            delete Global::wheelUpKeys;
            Global::wheelUpKeys = nullptr;
        }

        Global::isGlobalWheel = false;
        ui.checkBox_WheelSelf->setEnabled(true);
        ui.checkBox_WheelSelf->setChecked(false);

        setCurrentTableWidgetRowBold(false, false, true);
    }
}

// 当前窗口 键盘上下键换成鼠标滚轮
void MainDialog::on_checkBox_WheelSelf_stateChanged(int state) {
    if (state == Qt::Checked) {
        setCurrentTableWidgetRowBold(true, false);
    } else {
        setCurrentTableWidgetRowBold(false, false);
    }
}

// 刷新
void MainDialog::on_pushButton_Refresh_clicked() {
    QString msg = tr("%1\n%2")
        .arg(tr("There are some windows hidden by the bosskey."))
        .arg(tr("Before refresh the list, you should show them."));

    if (toContinue("Refresh", msg))
        refreshList();
}

// 显示窗口
void MainDialog::on_pushButton_Show_clicked() {
    Utils::SetActiveWindow(Global::CurrWnd->hnd);
}

// 重命名窗口
void MainDialog::on_action_Rename_triggered() {
    bool ok = false;
    QString newTitle = QInputDialog::getText(this, tr("Rename caption"), tr("Please input new caption: "), 
        QLineEdit::Normal, Global::CurrWnd->caption, &ok);

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
    QString msg = tr("%1\n%2")
        .arg(tr("This window is hidden by the bosskey."))
        .arg(tr("Before setup this item, you should show it."));
    if (!toContinue(Global::CurrWnd, "Setup", msg)) return;

    unSetupHotKey(Global::CurrWnd->setting.hotkey);
    QString combo = ui.comboBox_Action->currentText();

    // no hotkey
    if (combo != Global::NO_ACTION && ui.hotKeyEdit->keySequence() == QKeySequence::NoMatch) {
        QMessageBox::critical(this, tr("Error"), tr("You must set a hotkey for this action."));
        return;
    }
    // no action || error
    bool error;
    if (combo == Global::NO_ACTION || (error = !setupHotKey(ui.hotKeyEdit->keySequence()))) {
        Global::CurrWnd->setting.hotkey = QKeySequence::NoMatch;
        Global::CurrWnd->setting.wndAction = WndBKType::NO_ACTION;

        if (error) 
            QMessageBox::critical(this, "Error", tr("This hotkey %1 has been used.").arg(ui.hotKeyEdit->keySequence().toString()));

        setCurrentTableWidgetRowBold(false, true);
    } 
    else {
        // Success
        Global::CurrWnd->setting.hotkey = ui.hotKeyEdit->keySequence();
        Global::CurrWnd->setting.needActive = ui.checkBox_NeedActive->isChecked();
        Global::CurrWnd->setting.needMute = ui.checkBox_NeedMute->isChecked();
        if (combo == Global::SHOW_STATUSBAR_ICON)
            Global::CurrWnd->setting.wndAction = WndBKType::SHOW_STATUSBAR_ICON;
        else if (combo == Global::HIDE_STATUSBAR_ICON)
            Global::CurrWnd->setting.wndAction = WndBKType::HIDE_STATUSBAR_ICON;

        setCurrentTableWidgetRowBold(true, true);
    }
    on_tableWidget_currentCellChanged(ui.tableWidget->currentRow(), ui.tableWidget->currentColumn(), 0, 0);
}

// 删除状态
void MainDialog::on_pushButton_Delete_clicked() {
    if (Global::CurrWnd->setting.wndAction == WndBKType::HIDE_STATUSBAR_ICON) {
        QString msg = tr("%1\n%2")
            .arg(tr("This window is hidden by the bosskey."))
            .arg(tr("Before delete the setting, you should show it."));
        if (!toContinue(Global::CurrWnd, "Delete", msg))
            return;
    }
    ui.hotKeyEdit->setKeySequence(QKeySequence::NoMatch);
    ui.comboBox_Action->setCurrentIndex(0);
    unSetupHotKey(Global::CurrWnd->setting.hotkey);

    Global::CurrWnd->setting.hotkey = QKeySequence::NoMatch;
    Global::CurrWnd->setting.needActive = false;
    Global::CurrWnd->setting.needMute = false;
    Global::CurrWnd->setting.wndAction = WndBKType::NO_ACTION;

    setCurrentTableWidgetRowBold(false, true);
    on_tableWidget_currentCellChanged(ui.tableWidget->currentRow(), ui.tableWidget->currentColumn(), 0, 0);
}

// 显示所有隐藏
void MainDialog::on_pushButton_ShowHidden_clicked() {
    int cnt = 0;
    foreach (Wnd wnd, Global::WindowsList) {
        if (wnd.setting.wndAction == WndBKType::HIDE_STATUSBAR_ICON && IsWindow(wnd.hnd) && !IsWindowVisible(wnd.hnd)) {
            cnt++;
            Utils::UnHideWindow(wnd.hnd);
            Utils::SetMute(&wnd, false);
        }
    }
    if (cnt == 0)
        QMessageBox::information(this, "Show", "There is no window hidden.");
    else
        QMessageBox::information(this, "Show", (cnt == 1) ? "1 window shown succeeded." : tr("%1%2").arg(cnt).arg(" windows shown succeeded."));
}

// 窗口关闭
void MainDialog::closeEvent(QCloseEvent *e) {
    // 判断是否有被隐藏的窗口
    QString msg = tr("%1\n%2")
        .arg(tr("There are some windows hidden by the bosskey."))
        .arg(tr("Before close the program, you should show them."));
    if (!toContinue("Exit", msg))
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
        QStringList() 
            << tr("Handle") << tr ("K") << tr ("W") 
            << tr("Pid") << tr("Process") << tr("Caption")  
    );
    ui.tableWidget->verticalHeader()->setDefaultSectionSize(20);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    vector<Wnd> allWnds = Utils::getAllWindows();
    Global::WindowsList = allWnds;
    foreach (Wnd wnd, allWnds) {
        int row = ui.tableWidget->rowCount();
        ui.tableWidget->insertRow(row);
        ui.tableWidget->setItem(row, 0, new QTableWidgetItem(wnd.hndString()));
        ui.tableWidget->setItem(row, 1, new QTableWidgetItem(""));
        ui.tableWidget->setItem(row, 2, new QTableWidgetItem(""));

        ui.tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(wnd.pid)));
        ui.tableWidget->setItem(row, 4, new QTableWidgetItem(wnd.image));
        ui.tableWidget->setItem(row, 5, new QTableWidgetItem(wnd.caption));
    }

    ui.label_Title->setText(tr("A&ctive Windows: (All %1)").arg(allWnds.size()));
    ui.tableWidget->clearSelection();
}

// 注册热键
bool MainDialog::setupHotKey(QKeySequence key) {
    foreach (QxtGlobalShortcut *sc, Global::shortcuts) {
        if (sc->shortcut() == key)
            return false;
    }
    auto shortcut = new QxtGlobalShortcut(this);
    if (!shortcut->setShortcut(key)) 
        return false;

    connect(shortcut, &QxtGlobalShortcut::activated, [=]() {

        foreach (Wnd wnd, Global::WindowsList) {
            if (wnd.setting.wndAction == WndBKType::NO_ACTION || wnd.setting.hotkey != key) 
                continue;

            if (wnd.setting.wndAction == WndBKType::SHOW_STATUSBAR_ICON) {
                bool isShowing = !IsIconic(wnd.hnd);
                if (wnd.setting.needActive && wnd.hnd != GetForegroundWindow()) 
                    continue;

                // 显示：判断活动窗口，静音，最小化
                // 最小化：判断活动窗口，取消静音，显示

                if (wnd.setting.needMute) 
                    Utils::SetMute(&wnd, isShowing);
                if (isShowing)
                    Utils::MinimizeWindow(wnd.hnd);
                else
                    Utils::RestoreWindow(wnd.hnd);
            }
            else if (wnd.setting.wndAction == WndBKType::HIDE_STATUSBAR_ICON) {
                bool isShowing = IsWindowVisible(wnd.hnd);
                if (isShowing && wnd.setting.needActive && wnd.hnd != GetForegroundWindow()) 
                    continue;

                // 显示：判断活动窗口，静音，隐藏
                // 隐藏：取消静音，显示

                if (wnd.setting.needMute)
                    Utils::SetMute(&wnd, isShowing);
                if (!isShowing)
                    Utils::UnHideWindow(wnd.hnd);
                else 
                    Utils::HideWindow(wnd.hnd);
            }
        }
    });
    Global::shortcuts.push_back(shortcut);
    return true;
}

// 注销热键
void MainDialog::unSetupHotKey(QKeySequence key) {
    for (auto iter = Global::shortcuts.begin(); iter != Global::shortcuts.end();) {
        QxtGlobalShortcut *sc = *iter;
        if (sc->shortcut() == key) {
            sc->disconnect();
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
        sc->disconnect();
        Global::shortcuts.erase(iter);
        delete sc;
    }
    ui.checkBox_WheelAll->setChecked(false);
}

// 检查所有窗口是否被隐藏
// 若存在隐藏窗口，弹出判断并根据选项返回
bool MainDialog::toContinue(QString title, QString msg) {
    bool has = false;
    foreach (Wnd wnd, Global::WindowsList) {
        if (wnd.setting.wndAction ==  WndBKType::HIDE_STATUSBAR_ICON && // 窗口操作为隐藏图标
            IsWindow(wnd.hnd) && !IsWindowVisible(wnd.hnd)) // 窗口存在并且被隐藏
            has = true;
    }

    // 不存在隐藏窗口, True
    if (!has) {
        unSetupAllHotKey();
        return true;
    }

    // 存在隐藏窗口, 判断
    QMessageBox::StandardButton result = 
        QMessageBox::information(this, title, msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    // 不操作, False
    if (result == QMessageBox::No) 
        return false;
    
    // 刷新后操作, True
    unSetupAllHotKey();
    on_pushButton_ShowHidden_clicked();
    return true;
}

// 检查指定是否被隐藏，若被隐藏，弹出判断并根据选项返回
bool MainDialog::toContinue(Wnd *wnd, QString title, QString msg) {

    if (!IsWindow(wnd->hnd) || IsWindowVisible(wnd->hnd))
        // 不被隐藏, True
        return true;

    // 被隐藏, 判断
    QMessageBox::StandardButton result = 
        QMessageBox::information(this, title, msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    // 不操作, False
    if (result == QMessageBox::No) 
        return false;

    // 显示后操作, True
    Utils::UnHideWindow(wnd->hnd);
    Utils::SetMute(wnd, false);
    return true;
}

#pragma endregion