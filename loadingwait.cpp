#include "loadingwait.h"
#include "ui_loadingwait.h"
#include <QMovie>
#include <QDesktopWidget>

LoadingWait::LoadingWait(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoadingWait)
{
    ui->setupUi(this);
    this->raise();
    setAttribute(Qt::WA_TranslucentBackground);//背景透明
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowCloseButtonHint | Qt::Dialog);
    QMovie *movie = new QMovie(":/loading.gif");
    ui->wait_gif->setMovie(movie);
    movie->start();
    ui->content->setText(content);
}

LoadingWait::~LoadingWait()
{
    delete ui;
}
