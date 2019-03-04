#include "dialogsignrecord.h"
#include "ui_dialogsignrecord.h"
#include "http.h"
#include "QJsonParseError"
#include "QJsonDocument"
#include "QJsonValue"
#include "mainwindow.h"
#include "QTableWidgetItem"
#include "QMessageBox"
#include "imd5.h"
#include "common.h"

DialogSignRecord::DialogSignRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSignRecord)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(11);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());
    QString device=Common::readSN();
    this->sn=device;
    QStringList ccNames=Common::readCert();
    ui->ccNames->addItems(ccNames);
    QStringList header;
    header  << "bundle id" << "证书名称" << "过期时间" << "过期提示语" << "签名时间"<< "app名称" << "备注" << "是否推送" << "联系信息" << "特殊信息" <<"更新时间";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    Http *http = new Http(NULL);
    QString respData = http->get(HTTP_SERVER+"/appSign/record?device="+device);
    QJsonParseError jsonError;
    QJsonDocument parseDoc = QJsonDocument::fromJson(respData.toLocal8Bit(),&jsonError);
    if(parseDoc.isArray()){
        this->jsonArray = parseDoc.array();
        tableDataReload();
    }
}

DialogSignRecord::~DialogSignRecord()
{
    delete ui;
}

void DialogSignRecord::cellChange(int row,int colum){
    qDebug() << "行："+QString::number(row,10)+" 列："+QString::number(colum,10);
    if(colum==0||colum==1){
        return;
    }
//    QMessageBox::StandardButton flag=QMessageBox::information(NULL, "", "是否修改数据",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
//    if(flag==QMessageBox::Yes){

//    }
    Http *http = new Http(NULL);
    int id=this->jsonArray.at(row).toObject()["Id"].toInt();
    QString idStr = QString::number(id,10);
    QString expireTimeStampStr;
    QString warningMessage;
    QString remarks;
    QString connectInfo;
    QString specialInfo;
    QString appName;
    if(colum==2){
        QString expireTime=ui->tableWidget->item(row,colum)->text();
        qDebug() << "expireTime:"+expireTime;
        uint expireTimeStamp=QDateTime::fromString(expireTime,"yyyy-MM-dd hh:mm:ss").toTime_t();
        expireTimeStampStr=QString::number(expireTimeStamp);
    }
    if(colum==3){
        warningMessage=ui->tableWidget->item(row,colum)->text();
        qDebug() << "warningMessage:"+warningMessage;
    }

    if(colum==5){
        appName = ui->tableWidget->item(row,colum)->text();
        qDebug() << "appName:"+appName;
    }

    if(colum==6){
        remarks = ui->tableWidget->item(row,colum)->text();
        qDebug() << "remarks:"+remarks;
    }

    if(colum==8){
        connectInfo = ui->tableWidget->item(row,colum)->text();
        qDebug() << "connectInfo:"+connectInfo;
    }
    if(colum==9){
        specialInfo = ui->tableWidget->item(row,colum)->text();
        qDebug() << "specialInfo:"+specialInfo;
    }

    imd5 imd5;
    QString sign=imd5.encode(idStr,salt);
    QString url = HTTP_SERVER+"/appSign/update";
    qDebug() << "url ==> "+url;
    QJsonObject jsonObject;
    jsonObject["id"]=idStr;
    jsonObject["expireTime"]=expireTimeStampStr;
    jsonObject["warningMessage"]=warningMessage;
    jsonObject["remarks"]=remarks;
    jsonObject["sign"]=sign;
    jsonObject["connectInfo"]=connectInfo;
    jsonObject["specialInfo"]=specialInfo;
    jsonObject["appName"]=appName;
    QString result=http->post(url,jsonObject);
    if(result=="true"){
        QMessageBox::information(NULL, "", "数据修改成功");
        return;
    }
    QMessageBox::information(NULL, "", "数据修改失败");
}

void DialogSignRecord::on_search_clicked()
{
    QString bundleId=ui->bundleId->text();
    QString ccName=ui->ccNames->currentText();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();
    Http *http = new Http(NULL);
    QString url = HTTP_SERVER+"/appSign/search";
    qDebug() << "url:"+url;
    QJsonObject jsonObject;
    jsonObject["bundleId"]=bundleId;
    jsonObject["device"]=this->sn;
    jsonObject["ccName"]=ccName;
    jsonObject["displayName"]=ui->appName->text();
    jsonObject["connectInfo"]=ui->connectInfo->text();
    QString respData=http->post(url,jsonObject);
    QJsonParseError jsonError;
    QJsonDocument parseDoc = QJsonDocument::fromJson(respData.toLocal8Bit(),&jsonError);
    if(parseDoc.isArray()){
        this->jsonArray = parseDoc.array();
        tableDataReload();
    }
}

void DialogSignRecord::tableDataReload(){
    disconnect(ui->tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(cellChange(int,int)));
    for(int i=0;i<jsonArray.size();++i){
        int row=ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        QJsonObject obj=jsonArray.at(i).toObject();
        QTableWidgetItem *item0=new QTableWidgetItem();
        item0->setText(obj["BundleId"].toString());
        ui->tableWidget->setItem(row,0,item0);
        QTableWidgetItem *item1=new QTableWidgetItem();
        QString ccName=obj["CcName"].toString();
        ccName=ccName.mid(21);
        item1->setText(ccName);
        ui->tableWidget->setItem(row,1,item1);
        QTableWidgetItem *item2=new QTableWidgetItem();
        QString expireTime=obj["ExpireTime"].toString();
        if(expireTime=="0001-01-01 00:00:00"){
            item2->setText("");
            item2->setFlags(Qt::ItemIsEnabled);
        }else{
            item2->setText(expireTime);
        }

        ui->tableWidget->setItem(row,2,item2);
        QTableWidgetItem *item3=new QTableWidgetItem();
        QString warningMessage=obj["WarningMessage"].toString();
        if(warningMessage==""){
            item3->setText("");
            item3->setFlags(Qt::ItemIsEnabled);
        }else{
            item3->setText(warningMessage);
        }

        ui->tableWidget->setItem(row,3,item3);
        QTableWidgetItem *item4=new QTableWidgetItem();
        item4->setFlags(Qt::ItemIsEnabled);
        item4->setText(obj["CreateTime"].toString());
        ui->tableWidget->setItem(row,4,item4);
        QTableWidgetItem *item5=new QTableWidgetItem();
        item5->setText(obj["AppName"].toString());
        ui->tableWidget->setItem(row,5,item5);
        QTableWidgetItem *item6=new QTableWidgetItem();
//        item6->setFlags(Qt::ItemIsEnabled);
        item6->setText(obj["Remarks"].toString());
        ui->tableWidget->setItem(row,6,item6);
        QTableWidgetItem *item7=new QTableWidgetItem();
        item7->setFlags(Qt::ItemIsEnabled);
        item7->setText(QString::number(obj["IsPush"].toInt(),10));
        ui->tableWidget->setItem(row,7,item7);

        QTableWidgetItem *item8=new QTableWidgetItem();
        item8->setText(obj["ConnectInfo"].toString());
        ui->tableWidget->setItem(row,8,item8);

        QTableWidgetItem *item9=new QTableWidgetItem();
        item9->setText(obj["SpecialInfo"].toString());
        ui->tableWidget->setItem(row,9,item9);
        QTableWidgetItem *item10=new QTableWidgetItem();
        item10->setFlags(Qt::ItemIsEnabled);
        QString updateTime = obj["UpdateTime"].toString();
        if(updateTime=="0001-01-01 00:00:00"){
            item10->setText("");
        }else{
            item10->setText(updateTime);
        }
        ui->tableWidget->setItem(row,10,item10);
    }
    connect(ui->tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(cellChange(int,int)));
}
