#include "MainDialog.h"
#include "Utils.hpp"
#include <iostream>
#include <QtWidgets/QMessageBox>
using namespace std;

MainDialog::MainDialog(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);
	loadWindowsList();
}

MainDialog::~MainDialog() {

}

void MainDialog::on_button_Refresh_clicked() {
	loadWindowsList();
}

void MainDialog::loadWindowsList() {
	ui.listWidget->clear();
	ui.label_Window->setText("");
	foreach (Wnd wnd, Utils::getAllWindows()) {
		QString itemstr = QString("%1 %2 %3 %4")
			.arg(Utils::hwnd2string(wnd.hnd).c_str())
			.arg(wnd.pid)
			.arg(wnd.image.c_str())
			.arg(wnd.caption.c_str());
		ui.listWidget->addItem(itemstr);
	}
}

void MainDialog::on_listWidget_currentTextChanged(QString str) {
	QStringList sl = str.split(" ");
	sl.removeFirst();
	sl.removeFirst();
	sl.removeFirst();
	QString cap = sl.join(" ");

	QFontMetrics fontWidth(ui.label_Window->font());
	QString elipstr = fontWidth.elidedText(cap, Qt::ElideRight, 140);

	ui.label_Window->setText(elipstr);
	ui.label_Window->setToolTip(cap);
}