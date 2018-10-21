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
#include "file.h"
#include "http.h"
#include "logindialog.h"
#include "imd5.h"
#include "http.h"
#include "expirepage.h"
#include "QJsonParseError"
#include "QJsonDocument"
#include "QJsonObject"

//软件到期时间
const int expireTime = 1540137600;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString parentPath;
    QString tmp;
    QString ipaName;
    QString ipaPath;
    QString mobileProvisionPath;
    QStringList ccNames;
    //MachO文件名
    QString machOFileName;
    QString appName;
    QString optoolPath = "/tmp/optool";
    QString libisigntooldylibPath = "/tmp/libisigntoolhook.dylib";

private:
    Ui::MainWindow *ui;
    LoginDialog *loginDialog;
    QProcess *process = new QProcess(this);
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    void initial();
    void validate();

private slots:
    void on_selectIpaButton_clicked();
    void on_provisionButton_clicked();
    void readProcessData();
    void signIpa();
    void on_clearLog_clicked();
};

#endif // MAINWINDOW_H
