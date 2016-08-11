#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "ui_QRCodeDialog.h"

namespace Ui {
class qrcodeDialog;
}

class QRCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QRCodeDialog(QWidget *parent = 0);
    void stringToQR(QString s);
    ~QRCodeDialog();

private:
    Ui::qrcodeDialog *ui;
};

#endif // DIALOG_H
