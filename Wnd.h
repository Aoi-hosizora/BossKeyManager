#ifndef WND_H
#define WND_H

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
};

#endif // WND_H