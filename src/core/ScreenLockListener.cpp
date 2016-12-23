#include "ScreenLockListener.h"

#if defined(Q_OS_OSX)
#include "ScreenLockListenerMac.h"
#endif
#if defined(Q_OS_LINUX)
#include "ScreenLockListenerDBus.h"
#endif

ScreenLockListener::ScreenLockListener(QObject* parent):
    QObject(parent){
#if defined(Q_OS_OSX)
    m_mac_listener= ScreenLockListenerMac::instance();
    connect(m_mac_listener,SIGNAL(screenLocked()), this,SIGNAL(screenLocked()));
#endif
#if defined(Q_OS_LINUX)
    m_dbus_listener= new ScreenLockListenerDBus(this);
    connect(m_dbus_listener,SIGNAL(screenLocked()), this,SIGNAL(screenLocked()));
#endif
}

ScreenLockListener::~ScreenLockListener(){
}

void ScreenLockListener::onNotification() {
    Q_EMIT screenLocked();
}
