# KeePassX

This forked repository is used to extend functionality of KeepassX:

* Show/Hide main window by a system tray icon.
* Add DBUS support (usefull to manage databases automatically on events like login, logout, etc...)

Changes will be submited to original KeepassX by pull requests.

## In case of modification about MainWindow class (public method)

Launch following commands directly from src/gui directory:

Regenerate XML file for DBus

    qdbuscpp2xml -M -s MainWindow.h -o org.keepassx.MainWindow.xml

Regenerate Adaptor source files from DBus XML

    qdbusxml2cpp -c MainWindowAdaptor -a MainWindowAdaptor.h:MainWindowAdaptor.cpp org.keepassx.MainWindow.xml
