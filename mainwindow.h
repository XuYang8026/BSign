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
#include <QDragEnterEvent>
#include <QMimeData>
#include "QJsonParseError"
#include "QJsonDocument"
#include "QJsonObject"
#include "ithread.h"
#include "ipainfo.h"
#include "common.h"
#include "signutil.h"
#include "signconfig.h"
#include "ifile.h"

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
    QString mobileProvisionPath;
    QString uuid;
    QStringList ccNames;
    QString optoolPath = "/tmp/optool";
    QString libisigntooldylibPath = "/tmp/libisigntoolhook.dylib";
    //注入位置
    QString injectionPositionPath;

private:
    Ui::MainWindow *ui;
    QProcess *process = new QProcess(this);
    IpaInfo *ipaInfo;
    void initial();
    void validate();
    void uiReset();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void on_selectIpaButton_clicked();
    void on_provisionButton_clicked();
    void readProcessData();
    void signIpa();
    void setIpaInfo(IpaInfo *ipaInfo);
    void execPrint(QString content);
    void on_ccNames_currentIndexChanged(const QString &arg1);
    void on_isPushMobileProvision_stateChanged(int arg1);
    void on_thirdFileList_currentIndexChanged(const QString &arg1);
    void on_thirdFileList_currentIndexChanged(int index);
    void on_injectionPositionButton_clicked();
    void on_injection_clicked();

};

#endif // MAINWINDOW_H
