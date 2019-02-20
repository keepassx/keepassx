#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include "ScreenLockListenerDBus.h"
#include <iostream>

ScreenLockListenerDBus::ScreenLockListenerDBus(QObject *parent) : QObject(parent)
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    QDBusConnection systemBus = QDBusConnection::systemBus();
    std::cout << "connect org.gnome.SessionManager.Presence.StatusChanged " << std::endl;
    sessionBus.connect(
                "org.gnome.SessionManager", // service
                "/org/gnome/SessionManager/Presence", // path
                "org.gnome.SessionManager.Presence", // interface
                "StatusChanged", // signal name
                this, //receiver
                SLOT(gnomeSessionStatusChanged(uint)));

    std::cout << "connect org.freedesktop.login1.Manager.PrepareForSleep " << std::endl;
    systemBus.connect(
                "org.freedesktop.login1", // service
                "/org/freedesktop/login1", // path
                "org.freedesktop.login1.Manager", // interface
                "PrepareForSleep", // signal name
                this, //receiver
                SLOT(logindPrepareForSleep(bool)));

    std::cout << "connect com.canonical.Unity.Session.Locked " << std::endl;
    sessionBus.connect(
                "com.canonical.Unity", // service
                "/com/canonical/Unity/Session", // path
                "com.canonical.Unity.Session", // interface
                "Locked", // signal name
                this, //receiver
                SLOT(unityLocked()));

    /* Currently unable to get the current user session from login1.Manager
    QDBusInterface login1_manager_iface("org.freedesktop.login1", "/org/freedesktop/login1",
                              "org.freedesktop.login1.Manager", systemBus);
    if(login1_manager_iface.isValid()){
        qint64 my_pid = QCoreApplication::applicationPid();
        QDBusReply<QString> reply = login1_manager_iface.call("GetSessionByPID",static_cast<quint32>(my_pid));
        if (reply.isValid()){
            QString current_session = reply.value();
            std::cout << "current session is " << current_session.toStdString() << std::endl;
        } else {
            std::cout << reply.error().message().toStdString() << std::endl;
        }
    } else {
        std::cout << login1_manager_iface.lastError().message().toStdString() << std::endl;
    }
    */
}

void ScreenLockListenerDBus::gnomeSessionStatusChanged(uint status){
    std::cout << "org.gnome.SessionManager.Presence.StatusChanged " << status << std::endl;
    if(status != 0){
        Q_EMIT screenLocked();
    }
}

void ScreenLockListenerDBus::logindPrepareForSleep(bool beforeSleep){
    std::cout << "org.freedesktop.login1.Manager.PrepareForSleep " << beforeSleep << std::endl;
    if(beforeSleep){
        Q_EMIT screenLocked();
    }
}

void ScreenLockListenerDBus::unityLocked(){
    std::cout << "com.canonical.Unity.Session.Locked " << std::endl;
    Q_EMIT screenLocked();
}
