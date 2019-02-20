#ifndef SCREENLOCKLISTENERWIN_H
#define SCREENLOCKLISTENERWIN_H

#include <QWidget>
#include <QAbstractNativeEventFilter>

class ScreenLockListenerWin : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit ScreenLockListenerWin(QWidget *parent = 0);
    ~ScreenLockListenerWin();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void screenLocked();

private:
    void * m_powernotificationhandle;
};

#endif // SCREENLOCKLISTENERWIN_H
