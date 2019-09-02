#ifndef WND_H
#define WND_H

#include <Windows.h>
using namespace std;


class Wnd {
public:
	HWND hnd;
	string caption;
	int pid;
	string image;

	Wnd(HWND hnd, string caption, int pid, string image)
		: hnd(hnd), caption(caption), pid(pid), image(image) {}
};

#endif // WND_H