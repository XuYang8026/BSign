#include "batchsupplementsign.h"
#include "ui_batchsupplementsign.h"
#include <QFileDialog>

BatchSupplementSign::BatchSupplementSign(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BatchSupplementSign)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());
    this->setWindowTitle("批量补签");
    QStringList ccNames = Common::readCert();
    QStringList ccNameList;
    ccNameList.append("请选择证书");
    ccNameList.append(ccNames);
    ui->ccNameComboBox->addItems(ccNameList);
    ui->expaire->setDateTime(QDateTime::currentDateTime());
}

BatchSupplementSign::~BatchSupplementSign()
{
    delete ui;
}

void BatchSupplementSign::on_batchSelectFiles_clicked()
{
    ui->isSelectSignFileList->setPlainText("");
    QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("open file"), "",  tr("file(*.ipa)"));
    for(QString filePath:filePaths){
        ui->isSelectSignFileList->appendPlainText(filePath);
        this->signFilePaths.append(filePath);
    }
    this->signFilePaths=filePaths;
}

void BatchSupplementSign::on_batchSelectFile_clicked()
{
    ui->isSelectSignFileList->setPlainText("");
    QString filePath=QFileDialog::getExistingDirectory(this, desktopPath);
    QFileInfoList fileInfoList=GetFileList(filePath);
    this->signFilePaths.clear();
    for(QFileInfo fileInfo:fileInfoList){
        if(fileInfo.suffix()!="ipa"){
            continue;
        }
        QString filePath=fileInfo.filePath();
        this->signFilePaths.append(filePath);
        ui->isSelectSignFileList->appendPlainText(filePath);
    }
}

void BatchSupplementSign::on_selectOutResignButton_clicked()
{
    QString filePath=QFileDialog::getExistingDirectory(this, desktopPath);
    ui->outResignPath->setText(filePath);
}

void BatchSupplementSign::execPrint(QString content){
    this->ui->execResult->appendPlainText(content);
}

void BatchSupplementSign::on_selectMobileProvisionPath_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), desktopPath,  tr("file(*.mobileprovision)"));
    ui->mobileProvisionPath->setText(filePath);
}

void BatchSupplementSign::on_startSign_clicked()
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
        AppSign appSign=Common::getAppSign(signUtil->ipaInfo->bundleId);

        if(appSign.id<=0){
            QMessageBox::warning(this, tr("QMessageBox::information()"),filePath+" 未读取到签名记录 不能执行补签操作");
            continue;
        }

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
        jsonObj.insert("remark",ui->remarks->document()->toPlainText());
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

SignConfig * BatchSupplementSign::readCurrentSignConfig(){
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

void BatchSupplementSign::on_isPushMobileProvision_stateChanged(int arg1)
{
    QString ccName=ui->ccNameComboBox->currentText();
    if(ccName=="请选择证书"|| ccName.isEmpty()){
        return;
    }
    mobileProvisionPath=Common::getMobileProvisionPath(ccName,ui->isPushMobileProvision->isChecked());
    if(mobileProvisionPath.isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未读取到"+ccName+"相关描述文件\n请手动选择");
    }
    ui->mobileProvisionPath->setText(mobileProvisionPath);
}

void BatchSupplementSign::on_ccNameComboBox_currentIndexChanged(const QString &arg1)
{
    if(ui->ccNameComboBox->currentText()=="请选择证书"){
        return;
    }
    mobileProvisionPath=Common::getMobileProvisionPath(arg1,ui->isPushMobileProvision->isChecked());
    if(mobileProvisionPath.isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未读取到"+arg1+"相关描述文件\n请手动选择");
    }
    ui->mobileProvisionPath->setText(mobileProvisionPath);
}
