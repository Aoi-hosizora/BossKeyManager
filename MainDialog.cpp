#include "MainDialog.h"
#include "Utils.hpp"
#include "Global.hpp"

#include <iostream>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QKeySequenceEdit>
#include <QtWidgets/QCheckBox>
#include <QCloseEvent>
#include "qxtglobalshortcut.h"

using namespace std;

MainDialog::MainDialog(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);
	setWindowFlags(Qt::Window
		| Qt::WindowCloseButtonHint
		| Qt::WindowMinimizeButtonHint);
	setFixedSize(this->width(), this->height());

	this->setWindowTitle(Global::WND_TITLE);
	onLoaded();
}

MainDialog::~MainDialog() {

}

#pragma region UI Interaction

// 窗口载入完毕
void MainDialog::onLoaded() {
	ui.comboBox_Action->insertItems(0, QStringList() 
		<< Global::NO_ACTION 
		<< Global::SHOW_STATUSBAR_ICON 
		<< Global::HIDE_STATUSBAR_ICON);
	loadWindowsList();
}

// QLabel 超过范围显示省略号
void MainDialog::setEclipseLabel(QLabel *label, QString str, int width) {
	QFontMetrics fontWidth(label->font());
	QString elipstr = fontWidth.elidedText(str, Qt::ElideRight, width);
	label->setText(elipstr);
	label->setToolTip(str);
}

// 列表选中改变
void MainDialog::on_listWidget_currentRowChanged(int idx) {
	if (idx != -1) {
		Global::CurrWnd = &Global::WindowsList.at(idx);
		setEclipseLabel(ui.label_Caption, Global::CurrWnd->caption, 180);
		setEclipseLabel(ui.label_Process, Global::CurrWnd->image, 180);

		if (Global::CurrWnd->actionhk == WndBKType::SHOW_STATUSBAR_ICON)
			ui.comboBox_Action->setCurrentIndex(1);
		else if (Global::CurrWnd->actionhk == WndBKType::HIDE_STATUSBAR_ICON)
			ui.comboBox_Action->setCurrentIndex(2);
		else
			ui.comboBox_Action->setCurrentIndex(0);
		ui.hotKeyEdit->setKeySequence(Global::CurrWnd->hotkey);
		ui.checkBox_ActiveToHide->setChecked(Global::CurrWnd->needActive);		
		ui.checkBox_Mute->setChecked(Global::CurrWnd->mute);		
	}

	ui.groupBox->setEnabled(idx != -1);
	ui.pushButton_Delete->setEnabled(Global::CurrWnd->actionhk != WndBKType::NO_ACTION);
}

// 刷新
void MainDialog::on_pushButton_Refresh_clicked() {
	QString msg = QString("%1\n%2")
		.arg(tr("There are some windows hidden by the bosskey. \n"))
		.arg(tr("Before refresh the list, you should show them."));

	if (!checkWndHideContinue("Refresh", msg)) return;
	loadWindowsList();
}

// 显示窗口
void MainDialog::on_pushButton_Show_clicked() {
	Utils::SetFGWindow(Global::CurrWnd->hnd);
}

// 重命名窗口
void MainDialog::on_pushButton_Rename_clicked() {
	bool ok = false;
	QString newTitle = QInputDialog::getText(this, 
		tr("Rename caption"), 
		tr("Please input new caption:"), 
		QLineEdit::Normal, 
		Global::CurrWnd->caption, 
		&ok);
	if (ok && !newTitle.isEmpty()) {
		Utils::SetWindowName(Global::CurrWnd->hnd, newTitle);

		Global::CurrWnd->caption = newTitle;
		ui.listWidget->item(ui.listWidget->currentRow())
			->setText(Global::CurrWnd->toQString());

		on_listWidget_currentRowChanged(ui.listWidget->currentRow());
	}
}

// 设置状态
void MainDialog::on_pushButton_Setup_clicked() {
	QString msg = QString("%1\n%2")
		.arg(tr("This window is hidden by the bosskey. \n"))
		.arg(tr("Before setup this item, you should show it."));
	if (!checkWndHideContinue(Global::CurrWnd->hnd, "Setup", msg)) return;

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

	QListWidgetItem *curr = ui.listWidget->selectedItems().at(0);
	QFont b = curr->font();
	b.setBold(combo != Global::NO_ACTION);
	curr->setFont(b);

	on_listWidget_currentRowChanged(ui.listWidget->currentRow());
}

// 删除状态
void MainDialog::on_pushButton_Delete_clicked() {
	// 判断窗口是否隐藏
	if (Global::CurrWnd->actionhk == WndBKType::HIDE_STATUSBAR_ICON) {
		QString msg = QString("%1\n%2")
			.arg(tr("This window is hidden by the bosskey. \n"))
			.arg(tr("Before delete the setting, you should show it."));
		if (!checkWndHideContinue(Global::CurrWnd->hnd, "Delete", msg))
			return;
	}
	Global::CurrWnd->actionhk = WndBKType::NO_ACTION;
	ui.comboBox_Action->setCurrentIndex(0);

	unSetupHotKey(Global::CurrWnd->hotkey);
	Global::CurrWnd->hotkey = QKeySequence::NoMatch;
	ui.hotKeyEdit->setKeySequence(QKeySequence::NoMatch);
	Global::CurrWnd->needActive = false;
	Global::CurrWnd->mute = false;

	QListWidgetItem *curr = ui.listWidget->selectedItems().at(0);
	QFont b = curr->font();
	b.setBold(false);
	curr->setFont(b);

	on_listWidget_currentRowChanged(ui.listWidget->currentRow());
}

// 显示所有隐藏
void MainDialog::on_pushButton_ShowAllWindowsHidden_clicked() {
	int cnt = 0;
	foreach (Wnd wnd, Global::WindowsList) {
		if (wnd.actionhk == WndBKType::HIDE_STATUSBAR_ICON && IsWindow(wnd.hnd) && !IsWindowVisible(wnd.hnd)) {
			cnt++;
			Utils::UnHideWindow(wnd.hnd);
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
		.arg(tr("There are some windows hidden by the bosskey. \n"))
		.arg(tr("Before close the program, you should show them."));
	if (!checkWndHideContinue("Exit", msg))
		e->ignore();
	else
		e->accept();
}

#pragma endregion UI Interaction

///////////////////////////////////////////////////////

#pragma region Functions

// 加载列表内容
void MainDialog::loadWindowsList() {
	unSetupAllHotKey();
	ui.listWidget->clear();

	ui.label_Caption->setText(tr("Not selected"));
	ui.label_Process->setText(tr("Not selected"));
	ui.hotKeyEdit->setKeySequence(QKeySequence::NoMatch);
	ui.groupBox->setEnabled(false);
	ui.comboBox_Action->setCurrentIndex(0);

	vector<Wnd> allWnds = Utils::getAllWindows();
	foreach (Wnd wnd, allWnds) {
		ui.listWidget->addItem(wnd.toQString());
	}
	ui.label_Title->setText(QString(tr("A&ctive Windows: (All %1)")).arg(allWnds.size()));

	Global::WindowsList = allWnds;
}

// 注册热键
bool MainDialog::setupHotKey(QKeySequence key) {
	foreach (QxtGlobalShortcut *sc, Global::shortcuts) {
		if (sc->shortcut() == key)
			return true;
	}

	auto shortcut = new QxtGlobalShortcut(this);
	if (shortcut->setShortcut(key)) {
		connect(shortcut, &QxtGlobalShortcut::activated, [=]() {
			foreach (Wnd wnd, Global::WindowsList) {
				if (wnd.actionhk != WndBKType::NO_ACTION && wnd.hotkey == key) {
					// 状态栏显示 ----------------- 1
					if (wnd.actionhk == WndBKType::SHOW_STATUSBAR_ICON) {
						// 需要活动窗口去隐藏
						if (wnd.needActive && wnd.hnd != GetForegroundWindow()) 
							return;

						bool isStatus = IsIconic(wnd.hnd);
						if (wnd.mute) 
							Utils::SetMute(&wnd, !isStatus);
						if (isStatus)
							Utils::RestoreWindow(wnd.hnd);
						else
							Utils::MinimizeWindow(wnd.hnd);
					} 
					// 状态栏不显示 ----------------- 2
					else {
						bool isShow = IsWindowVisible(wnd.hnd);
						if (wnd.mute) 
							Utils::SetMute(&wnd, isShow);
						if (isShow) {
							// 需要活动窗口去隐藏
							if (wnd.needActive && wnd.hnd != GetForegroundWindow()) 
								return;
							Utils::HideWindow(wnd.hnd);
						}
						else // TODO 更好的方法
							Utils::UnHideWindow(wnd.hnd);
					}
				}
			}
		});
		Global::shortcuts.push_back(shortcut);
		return true;
	}
	else {
		QMessageBox::critical(this,
			"Error",
			"This hotkey " + key.toString() + " has been used.");
		return false;
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
bool MainDialog::checkWndHideContinue(QString title, QString msg) {
	bool has = false;
	foreach (Wnd wnd, Global::WindowsList) {
		if (wnd.actionhk ==  WndBKType::HIDE_STATUSBAR_ICON && // 窗口操作为隐藏图标
			IsWindow(wnd.hnd) && !IsWindowVisible(wnd.hnd)) // 该窗口还存在并且被隐藏
			has = true;
	}
	// 不存在隐藏窗口，继续操作
	if (!has) return true;

	// 存在隐藏窗口，判断
	QMessageBox::StandardButton result = QMessageBox::information(this,
		title, msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

	// 不操作
	if (result == QMessageBox::No) 
		return false;
	
	// 刷新后操作
	on_pushButton_ShowAllWindowsHidden_clicked();
	return true;
}

// 检查指定是否被隐藏
bool MainDialog::checkWndHideContinue(HWND hnd, QString title, QString msg) {
	if (IsWindow(hnd) && !IsWindowVisible(hnd)) {
		// 被隐藏，判断
		QMessageBox::StandardButton result = QMessageBox::information(this, 
			title, msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

		// 不操作
		if (result == QMessageBox::No) 
			return false;

		// 显示后操作
		Utils::UnHideWindow(hnd);
		return true;
	}
	// 不被隐藏
	return true;
}

#pragma endregion Functions