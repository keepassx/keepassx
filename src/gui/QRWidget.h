#ifndef QR_WIDGET_H
#define QR_WIDGET_H

#include <memory>
#include <functional>

#include <QWidget>
#include "ui_QRWidget.h"
#include <qrencode.h>
#include <QGraphicsScene>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QTimer>
#include <map>

class QTimer;
class QImage;
class QResizeEvent;


template <typename T>
using deleted_pointer = std::unique_ptr<T,std::function<void(T*)>>;

auto QRdeleter = [](QRcode* code){QRcode_free(code);};


QPixmap paintQRcode(const QRcode& code);





class QRWidget : public QWidget
{
  Q_OBJECT
  
public:
 QRWidget(QWidget* parent = nullptr);
 
public Q_SLOTS:
 void setUser(const QString& user);
 void setPass(const QString& pass);
 
 void setEntryName(const QString& title);
 
 void showUser();
 void showPass();
 
 void hideUser();
 void hidePass();
 
 void cancel();
 void accept();
 
 void clear();

Q_SIGNALS:
  void Finished(bool accepted);
  
 
 private:
   
  deleted_pointer<QRcode> get_qrcode(const QString& st);
  
  void scale_QRcodes();
  
  virtual void resizeEvent(QResizeEvent*);
  virtual void showEvent(QShowEvent*);
  virtual void timerEvent(QTimerEvent*);
   
  const std::unique_ptr<Ui::QRWidget> m_ui;
  
  deleted_pointer<QRcode> m_user_qr;
  deleted_pointer<QRcode> m_pass_qr;
  
  QRecLevel m_eclevel;
  
  int m_timer_msec;
  const int m_time_display_update_msec = 10;
  
  QGraphicsScene m_blank;
  QGraphicsScene m_user_scene;
  QGraphicsScene m_pass_scene;
  
  QTimer m_user_timer; 
  QTimer m_pass_timer;
  
  QGraphicsPixmapItem m_user_pixmap_item;
  QGraphicsPixmapItem m_pass_pixmap_item;
  
  QPixmap m_user_pixmap;
  QPixmap m_pass_pixmap;
  QPixmap m_blank_pixmap;
  
};

#endif
