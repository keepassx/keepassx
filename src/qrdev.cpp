
#include <QApplication>
#include "gui/QRWidget.h"
#include <string>

#include <iostream>

int main(int argc, char** argv)
{
  QApplication app(argc,argv);
  
  std::string testUser = "sdsuhdfasdfkljadlkbermnb";
  std::string testPass = "Oscar Wilde";
    
  QRWidget* widget = new QRWidget();
  
  app.setActiveWindow(static_cast<QWidget*>(widget));
//   test QR generation
  
  widget->setUser(testUser);
  widget->setPass(testPass);
  
  widget->show();
  
  
  
  return app.exec();



}