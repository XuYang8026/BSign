#include "logindialog.h"
#include "ui_logindialog.h"
#include "mainwindow.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("欢迎使用Isign-tool");
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginButton_clicked()
{
        this->hide();
        MainWindow *mainWindow=new MainWindow(this);
        mainWindow->show();

}
