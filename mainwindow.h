#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QString"
#include "QFileDialog"
#include <QDebug>
#include <QProcess>
#include <QDir>
#include "QMessageBox"
#include "QStandardPaths"
#include "http.h"
#include "logindialog.h"
#include "imd5.h"
#include "expirepage.h"
#include "QJsonParseError"
#include "QJsonDocument"
#include "QJsonObject"
#include "ithread.h"
#include "dialogsignrecord.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString sn;//mac序列号
    QString currentTime;
    QString expireTime;
    QString parentPath;
    QString tmp;
    QString deployAppName;
    QString ipaName;
    QString ipaPath;
    QString bundleId;
    QString mobileProvisionPath;
    QString uuid;
    QStringList ccNames;
    //MachO文件名
    QString machOFileName;
    QString appName;
    QString optoolPath = "/tmp/optool";
    QString libisigntooldylibPath = "/tmp/libisigntoolhook.dylib";
    Ui::MainWindow* getUI();

private:
    Ui::MainWindow *ui;
    LoginDialog *loginDialog;
    QProcess *process = new QProcess(this);
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    void initial();
    void validate();
//    void readMachOFileNameAndAppNameAndBundleId(QString plistPath);

private slots:
    void on_selectIpaButton_clicked();
    void on_provisionButton_clicked();
    void readProcessData();
    void signIpa();
    void on_clearLog_clicked();
    void setBundleId(QString bundleId);
    void on_sign_record_clicked();
//    void on_fenfa_platform_clicked();
};

#endif // MAINWINDOW_H
