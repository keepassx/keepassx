#ifndef SCREENLOCKLISTENERDBUS_H
#define SCREENLOCKLISTENERDBUS_H

#include <QObject>

class ScreenLockListenerDBus : public QObject
{
    Q_OBJECT
public:
    explicit ScreenLockListenerDBus(QObject *parent = 0);

Q_SIGNALS:
    void screenLocked();

private Q_SLOTS:
    void gnomeSessionStatusChanged(uint status);
    void logindPrepareForSleep(bool beforeSleep);
    void unityLocked();
};

#endif // SCREENLOCKLISTENERDBUS_H
