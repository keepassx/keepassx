#ifndef SCREENLOCKLISTENER_H
#define SCREENLOCKLISTENER_H

#include <QObject>

#if defined(Q_OS_OSX)
class ScreenLockListenerMac;
#endif
class ScreenLockListener : public QObject {
    Q_OBJECT

public:
    ScreenLockListener(QObject* parent=NULL);
    ~ScreenLockListener();
    void onNotification();

Q_SIGNALS:
    void screenLocked();

private:
#if defined(Q_OS_OSX)
    ScreenLockListenerMac* m_mac_listener;
#endif
};

#endif // SCREENLOCKLISTENER_H
