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

SignConfig * BatchUpdate::readCurrentSignConfig(QString ccName,QString mobileProvision){
    SignConfig *signConfig = new SignConfig;
    signConfig->ccName=ccName;
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
    this->signConfig=signConfig;
    return signConfig;
}

void BatchUpdate::on_startSign_clicked()
{

    if(signFilePaths.length()<=0){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"请选择IPA文件");
        return;
    }
    int successNum=0;
    ui->execResult->appendPlainText("开始签名...");
    for(QString filePath:signFilePaths){
        ui->execResult->appendPlainText("");
        ui->execResult->appendPlainText("");
        ui->execResult->appendPlainText("正在进行 "+filePath+" 签名");
        SignUtil *signUtil = new SignUtil(this);
        signUtil->readIpaInfo(filePath);
        AppSign appSign=Common::getAppSign(signUtil->ipaInfo->bundleId);
        bool isPush=appSign.isPush==1?true:false;
        QString mobileProvisionPath=Common::getMobileProvisionPath(appSign.ccName,isPush);
        this->readCurrentSignConfig(appSign.ccName,mobileProvisionPath);

        signConfig->expireTime=appSign.expireTime;
        signConfig->useAppCount=true;

        connect(signUtil,SIGNAL(execPrint(QString)),this,SLOT(execPrint(QString)));
        bool res=signUtil->sign(signUtil->ipaInfo,signConfig);
        if(!res){
            ui->execResult->appendHtml("<span style='color:red'>"+filePath+" 文件签名失败！</span>");
            continue;
        }
        QString bundleId=signUtil->ipaInfo->bundleId;

        QString url=HTTP_SERVER+"/appSign";
        QJsonObject jsonObj;
        jsonObj.insert("uuid",signConfig->ccUuid);
        jsonObj.insert("bundleId",bundleId);
        jsonObj.insert("device",Common::readSN());
        jsonObj.insert("ccName",appSign.ccName);
        jsonObj.insert("appName",signUtil->ipaInfo->deployAppName);
        jsonObj.insert("isPush",QString::number(appSign.isPush));
        jsonObj.insert("connectInfo",appSign.connectInfo);
        jsonObj.insert("specialInfo",appSign.specialInfo);
        jsonObj.insert("warningMessage",appSign.warningMessage);
        jsonObj.insert("expireTime",appSign.expireTime);
        jsonObj.insert("remark",appSign.remarks);
        jsonObj.insert("warningType",QString::number(appSign.warningType));
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

void BatchUpdate::on_selectOutResignPathButton_clicked()
{
    QString filePath=QFileDialog::getExistingDirectory(this, desktopPath);
    ui->outResignPath->setText(filePath);
}

void BatchUpdate::execPrint(QString content){
    this->ui->execResult->appendPlainText(content);
}

void BatchUpdate::on_batchSelectIPAFile_clicked()
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
