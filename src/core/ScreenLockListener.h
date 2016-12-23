#ifndef SCREENLOCKLISTENER_H
#define SCREENLOCKLISTENER_H

#include <QWidget>

#if defined(Q_OS_OSX)
class ScreenLockListenerMac;
#endif
#if defined(Q_OS_LINUX)
class ScreenLockListenerDBus;
#endif
#if defined(Q_OS_WIN)
class ScreenLockListenerWin;
#endif
class ScreenLockListener : public QObject {
    Q_OBJECT

public:
    ScreenLockListener(QWidget* parent=NULL);
    ~ScreenLockListener();
    void onNotification();

Q_SIGNALS:
    void screenLocked();

private:
#if defined(Q_OS_OSX)
    ScreenLockListenerMac* m_mac_listener;
#endif
#if defined(Q_OS_LINUX)
    ScreenLockListenerDBus* m_dbus_listener;
#endif
#if defined(Q_OS_WIN)
    ScreenLockListenerWin* m_win_listener;
#endif
};

#endif // SCREENLOCKLISTENER_H
