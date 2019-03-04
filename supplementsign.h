#ifndef SUPPLEMENTSIGN_H
#define SUPPLEMENTSIGN_H

#include <QDialog>
#include "signconfig.h"
#include "common.h"
#include "signutil.h"

namespace Ui {
class SupplementSign;
}

class SupplementSign : public QDialog
{
    Q_OBJECT

public:
    explicit SupplementSign(QWidget *parent = nullptr);
    ~SupplementSign();
    IpaInfo *ipaInfo;
    QStringList signFilePaths;
    SignConfig *signConfig;
    SignConfig * readCurrentSignConfig();
    QString mobileProvisionPath;
    void uiReset();

private slots:
    void execPrint(QString content);

    void on_selectIpaButton_clicked();

    void on_ccNames_currentIndexChanged(const QString &arg1);

    void on_isPushMobileProvision_stateChanged(int arg1);

    void on_supplementSignButton_clicked();

    void on_provisionButton_clicked();

    void setIpaInfo(IpaInfo *ipaInfo);

private:
    Ui::SupplementSign *ui;
};

#endif // SUPPLEMENTSIGN_H
