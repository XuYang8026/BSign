#ifndef DIALOGSIGNRECORD_H
#define DIALOGSIGNRECORD_H

#include <QDialog>
#include "QJsonArray"

namespace Ui {
class DialogSignRecord;
}

class DialogSignRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSignRecord(QWidget *parent = 0);
    ~DialogSignRecord();
    QString sn;
    QJsonArray jsonArray;

private:
    Ui::DialogSignRecord *ui;
    void tableDataReload();

private slots:
    void cellChange(int row,int colum);
    void on_search_clicked();
};

#endif // DIALOGSIGNRECORD_H
