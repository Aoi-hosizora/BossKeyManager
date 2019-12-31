#ifndef WND_H
#define WND_H

#include <QtCore/QtCore>
#include <QtWidgets/QKeySequenceEdit>
#include <Windows.h>
using namespace std;

enum WndBKType {
    NO_ACTION, 
    SHOW_STATUSBAR_ICON, 
    HIDE_STATUSBAR_ICON
};

struct WndSetting {
    WndBKType wndAction;
    QKeySequence hotkey;
    bool needActive;
    bool needMute;
    bool wheelFunction;
};

class Wnd {
public:
    HWND hnd;
    QString caption;
    int pid;
    QString image;

    QString hndString() {
        return QString("0x%1").arg(QString("%1").arg((int) hnd, 6, 16, QLatin1Char('0')).toUpper());
    }

    WndSetting setting;

    Wnd(HWND hnd, QString caption, int pid, QString image) : hnd(hnd), caption(caption), pid(pid), image(image) {
        setting.wndAction = WndBKType::NO_ACTION;
        setting.hotkey = QKeySequence::NoMatch;
        setting.needActive = setting.needMute = false;
        setting.wheelFunction = false;
    }
};

#endif // WND_H