#include "expirepage.h"
#include "ui_expirepage.h"

expirepage::expirepage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::expirepage)
{
    ui->setupUi(this);
}

expirepage::~expirepage()
{
    delete ui;
}
