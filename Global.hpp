#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "Wnd.h"
#include <QtCore/QtCore>
#include "qxtglobalshortcut.h"

class Global {

public:
    static vector<Wnd> WindowsList;
    static Wnd *CurrWnd;
    static vector<QxtGlobalShortcut *> shortcuts;

    // String Const
    static QString WND_TITLE;
    static QString NO_ACTION;
    static QString SHOW_STATUSBAR_ICON;
    static QString HIDE_STATUSBAR_ICON;

};

vector<Wnd> Global::WindowsList = vector<Wnd>();
Wnd *Global::CurrWnd = nullptr;
vector<QxtGlobalShortcut *> Global::shortcuts = vector<QxtGlobalShortcut *>();

// String Const
QString Global::WND_TITLE = QObject::tr("BossKey Manager");
QString Global::NO_ACTION = QObject::tr("No action");
QString Global::SHOW_STATUSBAR_ICON = QObject::tr("Show StatusBar icon");
QString Global::HIDE_STATUSBAR_ICON = QObject::tr("Hide StatusBar icon");

#endif // GLOBAL_HPP
