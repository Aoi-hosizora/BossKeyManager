#ifndef WND_H
#define WND_H

#include "Utils.hpp"
#include <QtCore/QtCore>
#include <Windows.h>
using namespace std;

class Wnd {
public:
	HWND hnd;
	QString caption;
	int pid;
	QString image;

	Wnd(HWND hnd, QString caption, int pid, QString image)
		: hnd(hnd), caption(caption), pid(pid), image(image) {}

	Wnd() {}

	QString toQString() {
		QString h = QString("0x%1").arg(QString("%1").arg((int) hnd, 6, 16, QLatin1Char('0')).toUpper());
		QString itemstr = QString("%1 %2 %3 \"%4\"")
			.arg(h)
			.arg(pid)
			.arg(image)
			.arg(caption);
		return itemstr;
	}
};

#endif // WND_H