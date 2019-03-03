#ifndef BATCHUPDATE_H
#define BATCHUPDATE_H

#include <QDialog>
#include "common.h"
#include <QStandardPaths>
#include <QMessageBox>
#include "signutil.h"
#include "signconfig.h"
#include "http.h"
#include "ifile.h"
#include <QFileDialog>
#include "loadingwait.h"

namespace Ui {
class BatchUpdate;
}

class BatchUpdate : public QDialog
{
    Q_OBJECT

public:
    explicit BatchUpdate(QWidget *parent = 0);
    ~BatchUpdate();
    QStringList signFilePaths;
    SignConfig * readCurrentSignConfig(QString ccName,QString mobileProvision);

private slots:
    void execPrint(QString);
    void on_batchSelectFile_clicked();

    void on_startSign_clicked();

    void on_selectOutResignPathButton_clicked();

private:
    Ui::BatchUpdate *ui;
};

#endif // BATCHUPDATE_H
