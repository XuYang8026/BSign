#include "batchupdate.h"
#include "ui_batchupdate.h"

BatchUpdate::BatchUpdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BatchUpdate)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());
}

BatchUpdate::~BatchUpdate()
{
    delete ui;
}

void BatchUpdate::on_batchSelectFile_clicked()
{
    ui->isSelectSignFileList->setPlainText("");
    this->signFilePaths.clear();
    QString filePath=QFileDialog::getExistingDirectory(this, desktopPath);
    QFileInfoList fileInfoList=GetFileList(filePath);
    for(QFileInfo fileInfo:fileInfoList){
        if(fileInfo.suffix()!="ipa"){
            continue;
        }
        QString filePath=fileInfo.filePath();
        this->signFilePaths.append(filePath);
        ui->isSelectSignFileList->appendPlainText(filePath);
    }
}

SignConfig * BatchUpdate::readCurrentSignConfig(QString ccName,QString mobileProvision){
    SignConfig *signConfig = new SignConfig;
    signConfig->ccName=ccName;
    if(ui->setExpaire->isChecked()){
        signConfig->expireTime=QString::number(ui->expaire->dateTime().toTime_t(),10);
        signConfig->warningMessage=ui->warning_message->text();
    }
    if(ui->clickNibSign->isChecked()){
        signConfig->signNib=true;
    }

    if(ui->clickFrameworkSign->isChecked()){
        signConfig->signFramwork=true;
    }

    if(ui->clickDylibSign->isChecked()){
        signConfig->signDylib=true;
    }

    if(ui->openAppUseCount->isChecked()){
        signConfig->useAppCount=true;
    }
    signConfig->mobileProvisionPath=mobileProvision;
    return signConfig;
}

void BatchUpdate::on_startSign_clicked()
{

    if(signFilePaths.length()<=0){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"请选择IPA文件");
        return;
    }
    LoadingWait *loadingWait = new LoadingWait(this);
    loadingWait->show();
    for(QString filePath:signFilePaths){
        SignUtil *signUtil = new SignUtil(this);
        signUtil->readIpaInfo(filePath);
        AppSign appSign=Common::getAppSign(signUtil->ipaInfo->bundleId);
        bool isPush=appSign.isPush==1?true:false;
        QString mobileProvisionPath=Common::getMobileProvisionPath(appSign.ccName,isPush);
        SignConfig *signConfig=this->readCurrentSignConfig(appSign.ccName,mobileProvisionPath);
        connect(signUtil,SIGNAL(execPrint(QString)),this,SLOT(execPrint(QString)));
        bool res=signUtil->sign(signUtil->ipaInfo,signConfig);
        if(!res){
            ui->execResult->appendPlainText(filePath+" 文件签名失败！");
            loadingWait->close();
            return;
        }
        QString bundleId=signUtil->ipaInfo->bundleId;

        QString url=HTTP_SERVER+"/appSign";
        QJsonObject jsonObj;
        jsonObj.insert("uuid",signConfig->ccUuid);
        jsonObj.insert("bundleId",bundleId);
        jsonObj.insert("device",Common::readSN());
        jsonObj.insert("ccName",appSign.ccName);
        jsonObj.insert("appName",signUtil->ipaInfo->deployAppName);
        jsonObj.insert("isPush",appSign.isPush);
        jsonObj.insert("connectInfo",appSign.connectInfo);
        jsonObj.insert("specialInfo",appSign.specialInfo);
        jsonObj.insert("warningMessage",appSign.warningMessage);
        jsonObj.insert("expireTime",appSign.expireTime);
        jsonObj.insert("remarks",appSign.remarks);
        if(ui->setExpaire->isChecked()){
            QString warningMessage=ui->warning_message->text();
            int expireTimeStamp=ui->expaire->dateTime().toTime_t();
            jsonObj.insert("warningMessage",warningMessage);
            jsonObj.insert("expireTime",QString::number(expireTimeStamp,10));
        }

        if(!ui->connectInfo->text().isEmpty()){
            jsonObj.insert("connectInfo",ui->connectInfo->text());
        }
        if(!ui->specialInfo->text().isEmpty()){
            jsonObj.insert("specialInfo",ui->specialInfo->text());
        }
        QString remarks=ui->remarks->document()->toPlainText();
        if(!remarks.isEmpty()){
            jsonObj.insert("remark",remarks);
        }
        Http *http = new Http(NULL);
        qDebug() << "请求url："+url;
        QString result=http->post(url,jsonObj);
        if(result!="true"){
            QMessageBox::about(NULL, tr(""),"签名失败，请重新尝试");
            loadingWait->close();
            return;
        }
    }
    loadingWait->close();
}

void BatchUpdate::on_selectOutResignPathButton_clicked()
{
    QString filePath=QFileDialog::getExistingDirectory(this, desktopPath);
    ui->outResignPath->setText(filePath);
}

void BatchUpdate::execPrint(QString content){
    this->ui->execResult->appendPlainText(content);
}
