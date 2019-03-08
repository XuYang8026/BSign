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
}

BatchSupplementSign::~BatchSupplementSign()
{
    delete ui;
}

void BatchSupplementSign::on_batchSelectFiles_clicked()
{
    ui->isSelectSignFileList->setPlainText("");
    this->signFilePaths.clear();
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
    this->signFilePaths.clear();
    QString filePath=QFileDialog::getExistingDirectory(this, desktopPath);

    if(filePath==""){
        return;
    }

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
    int successNum=0;
    ui->execResult->appendPlainText("开始签名...");
    for(QString filePath:signFilePaths){
        ui->execResult->appendPlainText("");
        ui->execResult->appendPlainText("");
        ui->execResult->appendPlainText("正在进行 "+filePath+" 签名");
        SignUtil *signUtil = new SignUtil(this);
        signUtil->readIpaInfo(filePath);
        AppSign appSign=Common::getAppSign(signUtil->ipaInfo->bundleId);

        if(appSign.id<=0){
            ui->execResult->appendHtml("<span style='color:red'>"+filePath+" 未读取到签名记录 跳过补签操作</span>");
            continue;
        }

        connect(signUtil,SIGNAL(execPrint(QString)),this,SLOT(execPrint(QString)));
        readCurrentSignConfig();
        bool res=signUtil->sign(signUtil->ipaInfo,signConfig);
        if(!res){
            ui->execResult->appendPlainText(filePath+" 文件签名失败！");
            continue;
        }
        QString url=HTTP_SERVER+"/appSign";
        QJsonObject jsonObj;
        jsonObj.insert("uuid",signConfig->ccUuid);
        jsonObj.insert("bundleId",appSign.bundleId);
        jsonObj.insert("device",Common::readSN());
        jsonObj.insert("ccName",ui->ccNameComboBox->currentText());
        jsonObj.insert("appName",signUtil->ipaInfo->deployAppName);
        jsonObj.insert("isPush",ui->isPushMobileProvision->isChecked()?"1":"0");
        jsonObj.insert("connectInfo",appSign.connectInfo);
        jsonObj.insert("specialInfo",appSign.specialInfo);
        jsonObj.insert("warningMessage",appSign.warningMessage);
        jsonObj.insert("expireTime",appSign.expireTime);
        jsonObj.insert("remark",appSign.remarks);
        Http *http = new Http(NULL);
        qDebug() << "请求url："+url;
        QString result=http->post(url,jsonObj);
        if(result!="true"){
            ui->execResult->appendHtml("<span style='color:red'>"+filePath+" 文件签名失败！</span>");
            continue;
        }
        successNum++;
    }
    ui->execResult->appendPlainText("批量签名 共"+QString::number(signFilePaths.size())+"个，成功"+QString::number(successNum)+"个");
}

SignConfig * BatchSupplementSign::readCurrentSignConfig(){
    SignConfig *signConfig = new SignConfig;
    signConfig->ccName=ui->ccNameComboBox->currentText();
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
    if(ui->isPushMobileProvision->isChecked()){
        signConfig->isPushMobileProvsion=true;
    }
    signConfig->mobileProvisionPath=ui->mobileProvisionPath->text();
    this->signConfig=signConfig;
    return signConfig;
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
