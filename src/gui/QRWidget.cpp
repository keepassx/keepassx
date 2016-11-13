#include "QRWidget.h"

#include <QTimer>
#include <qdebug.h>
#include <QImage>
#include <QPainter>

#include <iostream>
#include <iomanip>
#include <type_traits>
#include <QDialog>
#include <cmath>

#include "core/Config.h"

QPixmap paintQRcode(const QRcode& code)
{
  QPixmap pmap(2*code.width+1,2*code.width+1);
  QPainter painter(&pmap);
  QBrush brush(Qt::black,Qt::SolidPattern);
  QPen pen(brush,0);
  painter.setPen(pen);
  painter.setBrush(brush);
  
  pmap.fill(Qt::white);
  
  for(int i = 0; i < code.width; i++)
    for(int j= 0; j < code.width; j++)
    {
      //extract data byte of image
      int dat =  (*(code.data + i*code.width + j) & 0b1) ;
      if(dat)
	painter.drawRect(2*i,2*j,1,1);
    }
  
  return pmap;
}

void printQRcode(const QRcode& code)
{
 for(int i =0 ; i  < code.width; i++)
 {
   for(int j = 0; j < code.width; j++)
   {
     bool dat = (*(code.data + i * code.width + j) & 1);
     if(dat)
       std::cout << "+" ;
     else
       std::cout << " ";
       
   }
   std::cout << std::endl;
 }
  
}



QRWidget::QRWidget(QWidget* parent): QWidget(parent), m_ui( new Ui::QRWidget() ),
m_blank(this), m_user_scene(this), m_pass_scene(this)
{
  
  m_ui->setupUi(this);
  
  m_blank_pixmap.fill();
  m_blank.addPixmap(m_blank_pixmap);
  
  m_user_scene.addItem(&m_user_pixmap_item);
  m_pass_scene.addItem(&m_pass_pixmap_item);

  m_ui->PassDisplay->setScene(&m_blank);
  m_ui->UserDisplay->setScene(&m_blank);
  
  auto titleFont = m_ui->entryTitle->font();
  titleFont.setBold(true);
  titleFont.setPointSize(titleFont.pointSize() + 2);
  m_ui->entryTitle->setFont(titleFont);
  
  
  connect(m_ui->showPassButton, &QPushButton::pressed, this, &QRWidget::showPass);
  connect(m_ui->showUserButton, &QPushButton::pressed, this, &QRWidget::showUser);
  
  connect(&m_pass_timer, &QTimer::timeout, this, &QRWidget::hidePass);
  connect(&m_user_timer, &QTimer::timeout, this, &QRWidget::hideUser);
 
  connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QRWidget::accept);
  connect(m_ui->buttonBox, &QDialogButtonBox::rejected,this,&QRWidget::cancel);
  
  
  double timeout_sec = config()->get("QR/timeout").toDouble();
  m_timer_msec = std::round(timeout_sec * 1000);
  qDebug() << "m_timer_msec: "<< m_timer_msec;
  
  //NOTE: indices in the settings combobox chosen to correctly map onto QR_ECLEVEL enum
  m_eclevel = static_cast<QRecLevel>(config()->get("QR/errorcorrection").toInt());
  
  startTimer(m_time_display_update_msec);
  
}

void QRWidget::setPass(const QString& pass)
{
  m_pass_qr = get_qrcode(pass);
  Q_ASSERT(m_pass_qr);
  

  //WARNING: may not work because pixmap is temporary
  m_pass_pixmap = paintQRcode(*m_pass_qr);
  m_pass_pixmap_item.setPixmap(m_pass_pixmap);
  
//   m_ui->PassDisplay->setScene(&m_pass_scene);
  qDebug() << "bounding rect: " << m_pass_scene.itemsBoundingRect();
}


void QRWidget::setUser(const QString& user)
{
  m_user_qr = get_qrcode(user);
  Q_ASSERT(m_user_qr);
  
  
  m_user_pixmap = paintQRcode(*m_user_qr);
  m_user_pixmap_item.setPixmap(m_user_pixmap);
  
//   m_ui->UserDisplay->setScene(&m_user_scene);
}

void QRWidget::setEntryName(const QString& title)
{
  
  m_ui->entryTitle->setText(title);
  

}


void QRWidget::scale_QRcodes()
{
  if(m_ui->UserDisplay->scene() == &m_user_scene)
  {
    m_ui->UserDisplay->fitInView(m_user_scene.itemsBoundingRect(), Qt::KeepAspectRatio);
  }
  if(m_ui->PassDisplay->scene() == &m_pass_scene)
  {
    m_ui->PassDisplay->fitInView(m_pass_scene.itemsBoundingRect(),Qt::KeepAspectRatio);
  }
}



deleted_pointer< QRcode > QRWidget::get_qrcode(const QString& st)
{
  static const int qr_version = 0;
  static const int casesens = 1;
  
  deleted_pointer<QRcode> qrptr(
    QRcode_encodeString(st.toStdString().c_str(),qr_version, m_eclevel, QR_MODE_8, casesens),
    QRdeleter  );

  return qrptr;
}


void QRWidget::resizeEvent(QResizeEvent*)
{
    scale_QRcodes();
}

void QRWidget::showEvent(QShowEvent*)
{
    scale_QRcodes();
}

void QRWidget::timerEvent(QTimerEvent*)
{
  
    double pass_remain_sec = m_pass_timer.remainingTime() / 1000.;
    double user_remain_sec = m_user_timer.remainingTime() / 1000.;
  
    m_ui->PassTimeout->display(QString("%1").arg(pass_remain_sec,0,'f',1));
    m_ui->UserTimeout->display(QString("%1").arg(user_remain_sec,0,'f',1));
    
}



void QRWidget::showPass()
{
  if(m_ui->PassDisplay->scene() == &m_blank)
  {
    m_ui->PassDisplay->setScene(&m_pass_scene);
    scale_QRcodes();
    m_pass_timer.start(m_timer_msec);
  }
  
}


void QRWidget::showUser()
{
  if(m_ui->UserDisplay->scene() == &m_blank)
  {
    m_ui ->UserDisplay->setScene(&m_user_scene);
    scale_QRcodes();
    m_user_timer.start(m_timer_msec);
  }
  
}

void QRWidget::hidePass()
{
  if(m_ui->PassDisplay->scene() == &m_pass_scene)
  {
    m_ui->PassDisplay->setScene(&m_blank);
  }

  m_pass_timer.stop();
  m_ui->PassTimeout->display(QString("%1").arg(m_timer_msec,0,'f',1));
}


void QRWidget::hideUser()
{
  if(m_ui->UserDisplay->scene() == &m_user_scene)
  {
    m_ui->UserDisplay->setScene(&m_blank);
  }
  
  m_user_timer.stop();
  m_ui->PassTimeout->display(QString("%1").arg(m_timer_msec,0,'f',1));
  
}


void QRWidget::accept()
{
  clear();
  Q_EMIT Finished(true);
}


void QRWidget::cancel()
{
  clear();
  Q_EMIT Finished(false);
}

void QRWidget::clear()
{
  qDebug() << "TODO: implement clearing" ;

}



