#include "MainDialog.h"
#include "Utils.hpp"
#include "Global.hpp"

#include <iostream>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QKeySequenceEdit>
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
	}

	ui.groupBox->setEnabled(idx != -1);
	ui.pushButton_Delete->setEnabled(Global::CurrWnd->actionhk != WndBKType::NO_ACTION);
}

// 刷新
void MainDialog::on_pushButton_Refresh_clicked() {
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
	}
	QListWidgetItem *curr = ui.listWidget->selectedItems().at(0);
	QFont b = curr->font();
	b.setBold(combo != Global::NO_ACTION);
	curr->setFont(b);

	on_listWidget_currentRowChanged(ui.listWidget->currentRow());
}

// 删除状态
void MainDialog::on_pushButton_Delete_clicked() {
	Global::CurrWnd->actionhk = WndBKType::NO_ACTION;
	ui.comboBox_Action->setCurrentIndex(0);

	unSetupHotKey(Global::CurrWnd->hotkey);
	Global::CurrWnd->hotkey = QKeySequence::NoMatch;
	ui.hotKeyEdit->setKeySequence(QKeySequence::NoMatch);

	QListWidgetItem *curr = ui.listWidget->selectedItems().at(0);
	QFont b = curr->font();
	b.setBold(false);
	curr->setFont(b);

	on_listWidget_currentRowChanged(ui.listWidget->currentRow());
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
					// 状态栏显示
					if (wnd.actionhk == WndBKType::SHOW_STATUSBAR_ICON) {
						if (IsIconic(wnd.hnd))
							Utils::SetFGWindow(wnd.hnd);
						else
							Utils::MinimizeWindow(wnd.hnd);
					} 
					// 状态栏不显示
					else {
						if (IsWindowVisible(wnd.hnd))
							Utils::HideWindow(wnd.hnd);
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

#pragma endregion Functions
