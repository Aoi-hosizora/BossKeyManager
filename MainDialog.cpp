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
	ui.label_Caption->setText(tr("Not selected"));
	ui.label_Process->setText(tr("Not selected"));

	vector<Wnd> allWnds = Utils::getAllWindows();
	foreach (Wnd wnd, allWnds) {
		QString itemstr = QString("%1 %2 %3 %4")
			.arg(Utils::hwnd2qstring(wnd.hnd))
			.arg(wnd.pid)
			.arg(wnd.image)
			.arg(wnd.caption);
		ui.listWidget->addItem(itemstr);
	}
	ui.label_Title->setText(QString(tr("A&ctive Windows: (All %1)")).arg(allWnds.size()));
}

void MainDialog::on_listWidget_currentTextChanged(QString str) {
	QStringList sl = str.split(" ");
	if (sl.length() <= 3) return;
	sl.removeFirst();
	sl.removeFirst();
	QString pro = sl.at(0);
	sl.removeFirst();
	QString cap = sl.join(" ");

	setEclipseLabel(ui.label_Caption, cap, 180);
	setEclipseLabel(ui.label_Process, pro, 180);
}

void MainDialog::setEclipseLabel(QLabel *label, QString str, int width) {
	QFontMetrics fontWidth(label->font());
	QString elipstr = fontWidth.elidedText(str, Qt::ElideRight, width);
	label->setText(elipstr);
	label->setToolTip(str);
}