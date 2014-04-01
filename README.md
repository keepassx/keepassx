In case of modification about MainWindow class (public method)
--------------------------------------------------------------

Launch following commands directly from src/gui directory:

Regenerate XML file for DBus

    qdbuscpp2xml -M -s MainWindow.h -o org.keepassx.MainWindow.xml

Regenerate Adaptor source files from DBus XML

    qdbusxml2cpp -c MainWindowAdaptor -a MainWindowAdaptor.h:MainWindowAdaptor.cpp org.keepassx.MainWindow.xml
