#include "batchrsign.h"
#include "ui_batchrsign.h"
#include <QFileDialog>

BatchRSign::BatchRSign(QWidget *parent) :
    QDialog (parent),
    ui(new Ui::BatchRSign)
{
    ui->setupUi(this);
    QStringList ccNames = Common::readCert();
    QStringList ccNameList;
    ccNameList.append("请选择证书");
    ccNameList.append(ccNames);
    ui->ccNameComboBox->addItems(ccNameList);
    ui->expaire->setDateTime(QDateTime::currentDateTime());
}

BatchRSign::~BatchRSign()
{
    delete ui;
}

void BatchRSign::on_batchSelectFile_clicked()
{
    ui->isSelectSignFileList->setPlainText("");
    QString filePath=QFileDialog::getExistingDirectory(this, desktopPath);
    QFileInfoList fileInfoList=GetFileList(filePath);
//    this->signFilePaths=filePaths;
    for(QFileInfo fileInfo:fileInfoList){
        if(fileInfo.suffix()!="ipa"){
            continue;
        }
        QString filePath=fileInfo.filePath();
        this->signFilePaths.append(filePath);
        ui->isSelectSignFileList->appendPlainText(filePath);
    }
}

void BatchRSign::on_startSign_clicked()
{
    if(ui->mobileProvisionPath->text().isEmpty()||ui->mobileProvisionPath->text().isEmpty()||ui->ccNameComboBox->currentText()=="请选择证书"||ui->ccNameComboBox->currentText().isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"mobileprovision路径或证书名称不能为空");
        return;
    }

    if(signFilePaths.length()<=0){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"请选择IPA文件");
        return;
    }
    this->readCurrentSignConfig();
    for(QString filePath:signFilePaths){
        SignUtil *signUtil = new SignUtil(this);
        signUtil->readIpaInfo(filePath);
        connect(signUtil,SIGNAL(execPrint(QString)),this,SLOT(execPrint(QString)));
        bool res=signUtil->sign(signUtil->ipaInfo,signConfig);
        if(!res){
            ui->execResult->appendPlainText(filePath+" 文件签名失败！");
            return;
        }
        QString bundleId=signUtil->ipaInfo->bundleId;
        QString warningMessage=ui->warning_message->text();
        int expireTimeStamp=ui->expaire->dateTime().toTime_t();

        QString url=HTTP_SERVER+"/appSign";
        QJsonObject jsonObj;
        jsonObj.insert("uuid",signConfig->ccUuid);
        jsonObj.insert("bundleId",bundleId);
        jsonObj.insert("device",Common::readSN());
        jsonObj.insert("ccName",ui->ccNameComboBox->currentText());
        jsonObj.insert("appName",signUtil->ipaInfo->deployAppName);
        jsonObj.insert("isPush",ui->isPushMobileProvision->isChecked()?1:0);
        jsonObj.insert("connectInfo",ui->connectInfo->text());
        jsonObj.insert("specialInfo",ui->specialInfo->text());
        if(ui->setExpaire->isChecked()){
            jsonObj.insert("warningMessage",warningMessage);
            jsonObj.insert("expireTime",QString::number(expireTimeStamp,10));
        }
        Http *http = new Http(NULL);
        qDebug() << "请求url："+url;
        QString result=http->post(url,jsonObj);
        if(result!="true"){
            QMessageBox::about(NULL, tr(""),"签名失败，请重新尝试");
            return;
        }
    }
}

SignConfig * BatchRSign::readCurrentSignConfig(){
    SignConfig *signConfig = new SignConfig;
    signConfig->ccName=ui->ccNameComboBox->currentText();
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
    signConfig->mobileProvisionPath=ui->mobileProvisionPath->text();
    this->signConfig=signConfig;
}

void BatchRSign::execPrint(QString content){
    this->ui->execResult->appendPlainText(content);
}

void BatchRSign::on_selectMobileProvisionPath_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), desktopPath,  tr("file(*.mobileprovision)"));
    ui->mobileProvisionPath->setText(filePath);
}

void BatchRSign::on_ccNameComboBox_currentIndexChanged(const QString &arg1)
{
    if(ui->ccNameComboBox->currentText()=="请选择证书"){
        return;
    }

    QString mobileProvisionPath=Common::getMobileProvisionPath(arg1,ui->isPushMobileProvision->isChecked());
    if(mobileProvisionPath.isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未读取到"+arg1+"相关描述文件\n请手动选择");
    }
    ui->mobileProvisionPath->setText(mobileProvisionPath);
}
