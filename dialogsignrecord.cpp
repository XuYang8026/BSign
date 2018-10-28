#include "dialogsignrecord.h"
#include "ui_dialogsignrecord.h"
#include "http.h"
#include "QJsonParseError"
#include "QJsonDocument"
#include "QJsonValue"
#include "mainwindow.h"
#include "QTableWidgetItem"
#include "QMessageBox"

DialogSignRecord::DialogSignRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSignRecord)
{
    ui->setupUi(this);
    MainWindow *mainWindow=(MainWindow*)parentWidget();
    ui->tableWidget->setColumnCount(5);
    QStringList header;
    header << "bundle id" << "证书名称" << "过期时间" << "过期提示语" << "签名时间";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    Http *http = new Http(NULL);
    QString respData = http->get(HTTP_SERVER+"/appSign/record?device="+mainWindow->sn);
    QJsonParseError jsonError;
    QJsonDocument parseDoc = QJsonDocument::fromJson(respData.toLocal8Bit(),&jsonError);
    if(parseDoc.isArray()){
        this->jsonArray = parseDoc.array();
        for(int i=0;i<jsonArray.size();++i){
            int row=ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);

            QJsonObject obj=jsonArray.at(i).toObject();
            QTableWidgetItem *item0=new QTableWidgetItem();
            item0->setFlags(Qt::ItemIsEnabled);
            item0->setText(obj["BundleId"].toString());
            ui->tableWidget->setItem(row,0,item0);
            QTableWidgetItem *item1=new QTableWidgetItem();
            item1->setFlags(Qt::ItemIsEnabled);
            item1->setText(obj["Uuid"].toString());
            ui->tableWidget->setItem(row,1,item1);
            QTableWidgetItem *item2=new QTableWidgetItem();
            item2->setText(obj["ExpireTime"].toString());
            ui->tableWidget->setItem(row,2,item2);
            QTableWidgetItem *item3=new QTableWidgetItem();
            item3->setText(obj["WarningMessage"].toString());
            ui->tableWidget->setItem(row,3,item3);
            QTableWidgetItem *item4=new QTableWidgetItem();
            item4->setFlags(Qt::ItemIsEnabled);
            item4->setText(obj["CreateTime"].toString());
            ui->tableWidget->setItem(row,4,item4);
        }

    }
    connect(ui->tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(cellChange(int,int)));
}

DialogSignRecord::~DialogSignRecord()
{
    delete ui;
}

void DialogSignRecord::cellChange(int row,int colum){
    QMessageBox::StandardButton flag=QMessageBox::information(NULL, "修改数据", "Content", QMessageBox::Yes, QMessageBox::No);
    if(flag==QMessageBox::Yes){
        qDebug() << "yes";
        return;
    }
        qDebug() << "no";
//        QString text;
//        if(colum==2){
//            text=this->jsonArray.at(row).toObject()["ExpireTime"].toString();
//        }
//        if(colum==3){
//            text=this->jsonArray.at(row).toObject()["WarningMessage"].toString();
//        }
//        ui->tableWidget->item(row,colum)->setText(text);

}
