#include "supplementsign.h"
#include "ui_supplementsign.h"

SupplementSign::SupplementSign(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SupplementSign)
{
    ui->setupUi(this);
}

SupplementSign::~SupplementSign()
{
    delete ui;
}
