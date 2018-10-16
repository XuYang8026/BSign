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

private:
    Ui::MainWindow *ui;
    QProcess *process = new QProcess(this);
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

private slots:
    void on_selectIpaButton_clicked();
    void on_provisionButton_clicked();
    void readProcessData();
    void signIpa();
    void on_clearLog_clicked();
};

#endif // MAINWINDOW_H
