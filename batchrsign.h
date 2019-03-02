#ifndef BATCHRSIGN_H
#define BATCHRSIGN_H

#include <QDialog>
#include "common.h"
#include <QStandardPaths>
#include <QMessageBox>
#include "signutil.h"
#include "signconfig.h"
#include "http.h"
#include "ifile.h"

namespace Ui {
class BatchRSign;
}

class BatchRSign : public QDialog
{
    Q_OBJECT

public:
    explicit BatchRSign(QWidget *parent = 0);
    ~BatchRSign();
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QStringList signFilePaths;
    SignConfig *signConfig;

    SignConfig * readCurrentSignConfig();

private slots:

    void on_batchSelectFile_clicked();

    void on_startSign_clicked();

    void execPrint(QString);

    void on_selectMobileProvisionPath_clicked();

private:
    Ui::BatchRSign *ui;
};

#endif // BATCHRSIGN_H
