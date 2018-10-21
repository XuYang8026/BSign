#include "mainwindow.h"
#include <QApplication>
#include "logindialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    LoginDialog loginDialog;
//    loginDialog.show();
    MainWindow w;
    w.show();

    return a.exec();
}
