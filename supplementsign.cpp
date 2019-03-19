#include "supplementsign.h"
#include "ui_supplementsign.h"
#include <QFileDialog>
#include "ithread.h"

SupplementSign::SupplementSign(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SupplementSign)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());
    this->setWindowTitle("补签");
    QStringList ccNames = Common::readCert();
    ui->ccNames->addItems(ccNames);
    ui->expaire->setDateTime(QDateTime::currentDateTime());
}

SupplementSign::~SupplementSign()
{
    delete ui;
}

void SupplementSign::execPrint(QString content){
    this->ui->execResult->appendPlainText(content);
}

SignConfig * SupplementSign::readCurrentSignConfig(){
    SignConfig *signConfig = new SignConfig;
    signConfig->ccName=ui->ccNames->currentText();
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
    signConfig->mobileProvisionPath=ui->provisionFilePath->text();
    this->signConfig=signConfig;
}

void SupplementSign::on_isPushMobileProvision_stateChanged(int arg1)
{
    QString ccName=ui->ccNames->currentText();
    if(ccName=="请选择证书"|| ccName.isEmpty()){
        return;
    }
    mobileProvisionPath=Common::getMobileProvisionPath(ccName,ui->isPushMobileProvision->isChecked());
    if(mobileProvisionPath.isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未读取到"+ccName+"相关描述文件\n请将描述文件复制到"+workspacePath+"/"+ccName.mid(21)+"目录下或手动选择");
    }
    ui->provisionFilePath->setText(mobileProvisionPath);
}

void SupplementSign::on_selectIpaButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), "",  tr("file(*.ipa)"));
    ui->filePath->setText(filePath);
    uiReset();
    if(filePath.trimmed()!=""){
        IThread *ithread = new IThread;
        ithread->filePath=filePath.trimmed();
        connect(ithread,SIGNAL(send(IpaInfo*)),this,SLOT(setIpaInfo(IpaInfo*)));
        ithread->start();

    }
}

void SupplementSign::on_ccNames_currentIndexChanged(const QString &arg1)
{
    if(ui->ccNames->currentText()=="请选择证书"){
        return;
    }
    mobileProvisionPath=Common::getMobileProvisionPath(arg1,ui->isPushMobileProvision->isChecked());
    if(mobileProvisionPath.isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未读取到"+arg1+"相关描述文件\n请将描述文件复制到"+workspacePath+"/"+arg1.mid(21)+"目录下或手动选择");
    }
    ui->provisionFilePath->setText(mobileProvisionPath);
}

void SupplementSign::on_supplementSignButton_clicked()
{
    if(ui->filePath->text().isEmpty()||ui->provisionFilePath->text().isEmpty()||ui->ccNames->currentText()=="请选择证书"||ui->ccNames->currentText().isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"ipa路径或mobileprovision路径或证书名称不能为空");
        return;
    }
    QString tmp=ipaInfo->tmpPath;
    QString appName=ipaInfo->appName;

    QString machOFileName=ipaInfo->machOFileName;
    QString ipaName=ipaInfo->ipaName;
    QString deployAppName=ipaInfo->deployAppName;

    SignConfig *signConfig = new SignConfig;
    signConfig->bundleId=ui->bundleId->text();
    if(ui->useBundleId->isChecked()){
        signConfig->useMobileProvsionBundleId=true;
    }
    signConfig->mobileProvisionPath=ui->provisionFilePath->text();
    signConfig->displayName=ui->displayName->text();
    signConfig->ccName=ui->ccNames->currentText();
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

    SignUtil *signUtil = new SignUtil(this);
    connect(signUtil,SIGNAL(execPrint(QString)),this,SLOT(execPrint(QString)));
    bool res=signUtil->sign(ipaInfo,signConfig);
    ui->filePath->setText("");
    if(!res){
        return;
    }
    QString bundleId=signConfig->bundleId;
    QString warningMessage=ui->warning_message->text();
    QString url=HTTP_SERVER+"/appSign";
    QJsonObject jsonObj;
    jsonObj.insert("uuid",signConfig->ccUuid);
    jsonObj.insert("bundleId",bundleId);
    jsonObj.insert("device",Common::readSN());
    jsonObj.insert("ccName",ui->ccNames->currentText());
    jsonObj.insert("appName",deployAppName);
    int isPush = ui->isPushMobileProvision->isChecked()?1:0;
    jsonObj.insert("isPush",QString::number(isPush));
    jsonObj.insert("connectInfo",ui->connectInfo->text());
    jsonObj.insert("specialInfo",ui->specialInfo->text());
    jsonObj.insert("remark",ui->remarks->document()->toPlainText());
    if(ui->setExpaire->isChecked()){
        jsonObj.insert("warningMessage",warningMessage);
        QString expireTime=ui->expaire->dateTime().toString("yyyy-MM-dd hh:mm:ss");
        jsonObj.insert("expireTime",expireTime);
        QString warningType=ui->alterType->isChecked()?"1":"2";
        jsonObj.insert("warningType",warningType);
    }
    Http *http = new Http(NULL);
    qDebug() << "请求url："+url;
    QString result=http->post(url,jsonObj);
    if(result!="true"){
        QMessageBox::about(NULL, tr(""),"签名失败，请重新尝试");
        return;
    }
}

void SupplementSign::on_provisionButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), workspacePath,  tr("file(*.mobileprovision)"));
    ui->provisionFilePath->setText(filePath);
}

void SupplementSign::setIpaInfo(IpaInfo *ipaInfo){
    this->ui->bundleId->setText(ipaInfo->bundleId);
    this->ui->displayName->setText(ipaInfo->deployAppName);
    this->ipaInfo=ipaInfo;

    AppSign appSign=Common::getAppSign(ipaInfo->bundleId);
    if(appSign.id>0){
        ui->supplementSignButton->setEnabled(true);
        ui->ccNames->setCurrentText(appSign.ccName);

        QString expireTime=appSign.expireTime;
        if(expireTime!="0001-01-01 00:00:00"){
            ui->setExpaire->setChecked(true);
            ui->expaire->setDateTime(QDateTime::fromString(expireTime,"yyyy-MM-dd hh:mm:ss"));
        }
        if(appSign.warningType==1){
            ui->alterType->setChecked(true);
        }else{
            ui->exitType->setChecked(true);
        }
        QString appName=appSign.appName;
        QString ccName=appSign.ccName;
        ui->displayName->setText(appName);
        int isPush=appSign.isPush;
        ui->isPushMobileProvision->setChecked(isPush==1?true:false);
        //读取描述文件
        bool push=isPush==1?true:false;
        mobileProvisionPath=Common::getMobileProvisionPath(ccName,push);
        if(mobileProvisionPath.isEmpty()){
            QMessageBox::warning(this, tr("QMessageBox::information()"),"未读取到"+ccName+"相关描述文件");
        }
        ui->provisionFilePath->setText(mobileProvisionPath);
        ui->warning_message->setText(appSign.warningMessage);
        ui->remarks->setPlainText(appSign.remarks);
        ui->connectInfo->setText(appSign.connectInfo);
        ui->specialInfo->setText(appSign.specialInfo);
    }else{
        ui->execResult->appendHtml("<span style='color:red'>当前是补签页面,未读取到签名记录 不能执行补签操作</span>");
        ui->supplementSignButton->setEnabled(false);
    }
}

void SupplementSign::uiReset(){
    ui->provisionFilePath->setText("");
    ui->isPushMobileProvision->setChecked(false);
    ui->ccNames->setCurrentText("请选择证书");
    ui->useBundleId->setChecked(false);
    ui->setExpaire->setChecked(false);
    ui->remarks->setPlainText("");
    ui->warning_message->setText(WARNING_MESSAGE);
    ui->connectInfo->setText("");
    ui->specialInfo->setText("");
    ui->displayName->setText("");
    ui->bundleId->setText("");
}
