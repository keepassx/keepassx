#include "QRCodeDialog.h"

#include "qqrencode.h"
#include <QPixmap>

QRCodeDialog::QRCodeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qrcodeDialog)
{
    ui->setupUi(this);
}

void QRCodeDialog::stringToQR(QString s) {
    QGraphicsScene *sc = new QGraphicsScene();

    QQREncode encoder;
    encoder.encode(s);
    QPixmap qrcode;
    qrcode.convertFromImage(encoder.toQImage(300));
    sc->addPixmap(qrcode);
    ui->view->setScene(sc);
}


QRCodeDialog::~QRCodeDialog()
{
    delete ui;
}
