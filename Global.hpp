#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "Wnd.h"

class Global {

public:
	static vector<Wnd> WindowsList;
	static Wnd *CurrWnd;

};

vector<Wnd> Global::WindowsList = vector<Wnd>();
Wnd *Global::CurrWnd = nullptr;

#endif // GLOBAL_HPP
