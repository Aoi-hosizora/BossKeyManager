#ifndef HOTKEYEDIT_H
#define HOTKEYEDIT_H

#ifdef WIN32  
#pragma execution_character_set("utf-8")  
#endif

#include <QKeySequenceEdit>

class HotKeyEdit : public QKeySequenceEdit {
    Q_OBJECT

public:
    HotKeyEdit(QWidget *parent) : QKeySequenceEdit(parent) {}
    ~HotKeyEdit() {}

protected:
    void keyPressEvent(QKeyEvent *pEvent) {
        QKeySequenceEdit::keyPressEvent(pEvent);
        QKeySequence seq(QKeySequence::fromString(keySequence().toString().split(", ").first()));
        setKeySequence(seq);
    }
};

#endif // HOTKEYEDIT_H