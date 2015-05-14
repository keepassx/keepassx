#ifndef KEEPASSX_AUTOTYPEUNLOCKDIALOG_H
#define KEEPASSX_AUTOTYPEUNLOCKDIALOG_H

#include <QAbstractItemModel>
#include <QObject>
#include <QDialog>

#include <gui/DatabaseTabWidget.h>

#include "core/Global.h"

class UnlockDatabaseWidget;
class Database;

class UnlockDatabaseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UnlockDatabaseDialog(QWidget *parent = Q_NULLPTR);
    void setDBFilename(const QString& filename);
    void clearForms();
    Database* database();

Q_SIGNALS:
    void unlockDone(bool);

public Q_SLOTS:
    void Done(bool r);

private:
    UnlockDatabaseWidget* const m_view;
    DatabaseManagerStruct m_dbStruct;
};

#endif // KEEPASSX_AUTOTYPEUNLOCKDIALOG_H
