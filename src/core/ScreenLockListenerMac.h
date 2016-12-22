#ifndef SCREENLOCKLISTENERMAC_H
#define SCREENLOCKLISTENERMAC_H
#include <QObject>

#if defined(Q_OS_OSX)
#include <CoreFoundation/CoreFoundation.h>
#endif //Q_OS_OSX

class ScreenLockListenerMac: public QObject {
    Q_OBJECT

public:
    static ScreenLockListenerMac* instance();

#if defined(Q_OS_OSX)
    static void notificationCenterCallBack(CFNotificationCenterRef /*center*/, void */*observer*/,
                            CFNotificationName /*name*/, const void */*object*/, CFDictionaryRef /*userInfo*/);
#endif //Q_OS_OSX

Q_SIGNALS:
    void screenLocked();

private:
    ScreenLockListenerMac(QObject* parent=NULL);
    void onSignalReception();

};
#endif //SCREENLOCKLISTENERMAC_H
