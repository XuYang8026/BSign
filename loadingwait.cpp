#include "loadingwait.h"
#include "ui_loadingwait.h"
#include <QMovie>
#include <QDesktopWidget>

LoadingWait::LoadingWait(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadingWait)
{
    ui->setupUi(this);
    setAttribute (Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);//背景透明
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowCloseButtonHint | Qt::Dialog);
//    setWindowModality(Qt::ApplicationModal);
    QMovie *movie = new QMovie(":/loading.gif");
    ui->wait_gif->setMovie(movie);
    movie->start();
    ui->content->setText(content);
}

LoadingWait::~LoadingWait()
{
    delete ui;
}
