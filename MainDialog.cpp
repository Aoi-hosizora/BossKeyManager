#include "MainDialog.h"
#include "Utils.hpp"
#include <iostream>
#include <QtWidgets/QMessageBox>
using namespace std;

MainDialog::MainDialog(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	foreach (Wnd wnd, Utils::getAllWindows()) 
		cout << wnd.hnd << ends << wnd.caption << ends << wnd.pid << endl;

}

MainDialog::~MainDialog() {

}