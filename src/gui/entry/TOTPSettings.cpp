#include "TOTPSettings.h"
#include "ui_TOTPSettings.h"

TOTPSettingsDialog::TOTPSettingsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::TOTPSettings) {
    ui->setupUi(this);
}

TOTPSettingsDialog::~TOTPSettingsDialog(){
    delete ui;
}



QString TOTPSettingsDialog::seed(){
    return ui->seedEdit->text();
}

void TOTPSettingsDialog::setSeed(QString value){
    ui->seedEdit->setText(value);
}
