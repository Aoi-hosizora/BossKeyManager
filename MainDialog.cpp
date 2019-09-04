#include "MainDialog.h"
#include "Utils.hpp"
#include "Global.hpp"

#include <iostream>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>

using namespace std;

MainDialog::MainDialog(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);
	setWindowFlags(Qt::Window
		| Qt::WindowCloseButtonHint
		| Qt::WindowMinimizeButtonHint);
	setFixedSize(this->width(), this->height());

	onLoaded();
}

MainDialog::~MainDialog() {

}

#pragma region UI Interaction

// 窗口载入完毕
void MainDialog::onLoaded() {
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
	}
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

#pragma endregion UI Interaction

///////////////////////////////////////////////////////

#pragma region Functions

// 加载列表内容
void MainDialog::loadWindowsList() {
	ui.listWidget->clear();
	ui.label_Caption->setText(tr("Not selected"));
	ui.label_Process->setText(tr("Not selected"));

	vector<Wnd> allWnds = Utils::getAllWindows();
	foreach (Wnd wnd, allWnds) {
		ui.listWidget->addItem(wnd.toQString());
	}
	ui.label_Title->setText(QString(tr("A&ctive Windows: (All %1)")).arg(allWnds.size()));

	Global::WindowsList = allWnds;
}	

#pragma endregion Functions
