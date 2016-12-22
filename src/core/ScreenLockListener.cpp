#include "ScreenLockListener.h"

#if defined(Q_OS_OSX)
#include "ScreenLockListenerMac.h"
#endif

ScreenLockListener::ScreenLockListener(QObject* parent):
    QObject(parent){
#if defined(Q_OS_OSX)
    m_mac_listener= ScreenLockListenerMac::instance();
    connect(m_mac_listener,SIGNAL(screenLocked()), this,SIGNAL(screenLocked()));
#endif
}

ScreenLockListener::~ScreenLockListener(){
}

void ScreenLockListener::onNotification() {
    Q_EMIT screenLocked();
}
