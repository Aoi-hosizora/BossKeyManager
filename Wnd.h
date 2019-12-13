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
    WndBKType actionhk;
    QKeySequence hotkey;
    bool needActive;
    bool needMute;
};

class Wnd {
public:
    HWND hnd;
    QString caption;
    int pid;
    QString image;
    WndSetting setting;

    Wnd(HWND hnd, QString caption, int pid, QString image) : hnd(hnd), caption(caption), pid(pid), image(image) {
        setting.actionhk = WndBKType::NO_ACTION;
        setting.hotkey = QKeySequence::NoMatch;
        setting.needActive = setting.needMute = false;
    }

    QString hndString() {
        return QString("0x%1").arg(QString("%1").arg((int) hnd, 6, 16, QLatin1Char('0')).toUpper());
    }

    QString toQString() {
        QString itemstr = QString("%1 %2 %3 \"%4\"")
            .arg(hndString()).arg(pid).arg(image).arg(caption);
        return itemstr;
    }
};

#endif // WND_H