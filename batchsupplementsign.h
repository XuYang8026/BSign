#ifndef BATCHSUPPLEMENTSIGN_H
#define BATCHSUPPLEMENTSIGN_H

#include <QDialog>
#include "signconfig.h"
#include "common.h"
#include "signutil.h"

namespace Ui {
class BatchSupplementSign;
}

class BatchSupplementSign : public QDialog
{
    Q_OBJECT

public:
    explicit BatchSupplementSign(QWidget *parent = nullptr);
    ~BatchSupplementSign();
    QStringList signFilePaths;
    SignConfig *signConfig;

    SignConfig * readCurrentSignConfig();

    QString mobileProvisionPath;

private slots:
    void on_batchSelectFiles_clicked();

    void on_batchSelectFile_clicked();

    void on_selectOutResignButton_clicked();

    void execPrint(QString content);

    void on_selectMobileProvisionPath_clicked();

    void on_startSign_clicked();

    void on_isPushMobileProvision_stateChanged(int arg1);

    void on_ccNameComboBox_currentIndexChanged(const QString &arg1);
private:
    Ui::BatchSupplementSign *ui;
};

#endif // BATCHSUPPLEMENTSIGN_H
