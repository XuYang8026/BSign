#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "batchsupplementsign.h"

QString readSN(){
    QStringList snParams;
    snParams << "-c";
    snParams << "ioreg -rd1 -c IOPlatformExpertDevice |  awk '/IOPlatformSerialNumber/ { print $3; }'";
    QProcess *p = new QProcess;
    p->start("/bin/bash",snParams);
    p->waitForFinished();
    QString sn=p->readAllStandardOutput().trimmed().replace("\"","");
    return sn;
}

QString findSpecialFileQprocessParamsHandle(QString params,QString param){
    if(params.isEmpty()){
        params="-name \""+param+"\" ";
    }else{
        params+="-o -name \""+param+"\" ";
    }
    return params;
}

void MainWindow::initial(){

    //授予可执行权限
    QString cmd = "rm -rf "+libisigntoolappcountFilePath+" "+optoolFilePath+" "+libisigntoolhookFilePath;
    int flag=system(cmd.toLocal8Bit().data());
    if (flag!=0){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"重要组件执行权限失败");
    }

    bool libisigntoolappcountCopy=QFile::copy(":/libisigntoolappcount.dylib",libisigntoolappcountFilePath);

    if(!libisigntoolappcountCopy){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未获取重要组件");
    }

    bool optoolCopy=QFile::copy(":/optool",optoolFilePath);

    if(!optoolCopy){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未获取重要组件");
    }

    bool libisigntoolhookCopy=QFile::copy(":/libisigntoolhook.dylib",libisigntoolhookFilePath);

    if(!libisigntoolhookCopy){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未获取重要组件");
    }

    cmd = "chmod +x /tmp/optool";
    flag=system(cmd.toLocal8Bit().data());
    if (flag!=0){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未获取到optool执行权限");
    }
    ui->expaire->setDateTime(QDateTime::currentDateTime());

    //初始化工作空间
    QDir workspaceDir(workspacePath);
    if(!workspaceDir.exists()){
        workspaceDir.mkdir(workspacePath);
    }
    for(QString ccName:this->ccNames){
        ccName=ccName.mid(21);
        QDir ccNameDir(workspacePath+"/"+ccName);
        if(!ccNameDir.exists()){
            ccNameDir.mkdir(workspacePath+"/"+ccName);
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    this->setWindowTitle("欢迎使用 BSign V"+APP_VERSION);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
    ui->flag->setStyleSheet("color:red;");
    qDebug() << desktopPath;

    this->ccNames=Common::readCert();
    ui->ccNames->addItems(this->ccNames);

    this->sn=readSN();
    qDebug() << "mac 序列号 ==> "+sn;
    connect(ui->signButton, SIGNAL(clicked()), this,SLOT(signIpa()));
    this->initial();
    this->validate();

    QLabel *locationLabel = new QLabel("序列号："+sn+"    有效期："+this->expireTime+"      author:Jackson      QQ:3536391351");
    locationLabel->setMinimumWidth(640);
    locationLabel->setAlignment(Qt::AlignCenter);
    this->statusBar()->addWidget(locationLabel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectIpaButton_clicked()
{

    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), "",  tr("ipa(*.ipa)"));
    qDebug() << "选择文件路径："+filePath;
    ui->filePath->setText(filePath);
    uiReset();
    if(filePath.trimmed()!=""){
        IThread *ithread = new IThread;
        ithread->filePath=filePath.trimmed();
        connect(ithread,SIGNAL(send(IpaInfo*)),this,SLOT(setIpaInfo(IpaInfo*)));
        ithread->start();

    }
}


void MainWindow::on_provisionButton_clicked()
{

    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), workspacePath,  tr("file(*.mobileprovision)"));
    qDebug() << "选择文件路径："+filePath;
    this->mobileProvisionPath=filePath;
    ui->provisionFilePath->setText(filePath);
}

void MainWindow::readProcessData(){
    ui->execResult->appendPlainText(process->readAllStandardOutput());
}

void MainWindow::signIpa(){

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
    if(ui->isPushMobileProvision->isChecked()){
        signConfig->isPushMobileProvsion=true;
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
    jsonObj.insert("device",this->sn);
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
    }
    Http *http = new Http(NULL);
    qDebug() << "请求url："+url;
    QString result=http->post(url,jsonObj);
    if(result!="true"){
        QMessageBox::about(NULL, tr(""),"签名失败，请重新尝试");
        return;
    }
}

void MainWindow::validate(){
    //    获取当前时间
    Http *iHttp = new Http(NULL);
    QJsonObject paramObj;
    paramObj["device"]=this->sn;
//    paramObj["app_version"]=APP_VERSION;
    imd5 md5;
    QString sign=md5.encode(this->sn+APP_VERSION,"44a160d3f98c8a913ca192c7a6222790");
    paramObj["sign"]=sign;
    QString respData = iHttp->post(HTTP_SERVER+"/deviceInfo",paramObj);
    QJsonParseError jsonError;
    QJsonDocument parseDoc = QJsonDocument::fromJson(respData.toLocal8Bit(),&jsonError);
    if(jsonError.error != QJsonParseError::NoError){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"无效数据");
        exit(0);
    }
    if(parseDoc.isObject()){
        QJsonObject jsonObj = parseDoc.object();
        QJsonValue jsonTime=jsonObj.take("time");
        this->currentTime=jsonTime.toString();
        qDebug() << "系统时间:"+currentTime;
        QJsonValue jsonExpireTime=jsonObj.take("expireTime");
        this->expireTime=jsonExpireTime.toString();
        qDebug() << "有效时间:"+expireTime;
        QJsonValue jsonSign=jsonObj.take("sign");
        QString sign=jsonSign.toString();
        qDebug() << "sign:"+sign;
        imd5 md5;
        qDebug() << md5.encode(expireTime,"44a160d3f98c8a913ca192c7a6222790");
        if(md5.encode(expireTime,"44a160d3f98c8a913ca192c7a6222790")!=sign){
            QMessageBox::warning(this, tr("QMessageBox::information()"),"无效数据");
            exit(0);
        }
    }

    QDateTime timestamp = QDateTime::fromString(this->currentTime,"yyyy-MM-dd hh:mm:ss");
    QDateTime expireTimeStamp = QDateTime::fromString(this->expireTime,"yyyy-MM-dd hh:mm:ss");
    qDebug() << timestamp.toTime_t();
    if(timestamp.toTime_t() > expireTimeStamp.toTime_t()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"软件已过期\n请联系QQ:3536391351");
        exit(0);
    }
}


//void MainWindow::on_clearLog_clicked()
//{
//    ui->execResult->setPlainText("");
//}

void MainWindow::setIpaInfo(IpaInfo *ipaInfo){
    this->ui->bundleId->setText(ipaInfo->bundleId);
    this->ui->displayName->setText(ipaInfo->deployAppName);
    this->ipaInfo=ipaInfo;

    AppSign appSign=Common::getAppSign(ipaInfo->bundleId);
    if(appSign.id>0){
        ui->ccNames->setCurrentText(appSign.ccName);
        ui->flag->setText("更新");
        QString expireTime=appSign.expireTime;
        if(expireTime!="0001-01-01 00:00:00"){
            ui->setExpaire->setChecked(true);
            ui->expaire->setDateTime(QDateTime::fromString(expireTime,"yyyy-MM-dd hh:mm:ss"));
            ui->warning_message->setText(appSign.warningMessage);
        }

        QString ccName=appSign.ccName;

//        QString appName=appSign.appName;
//        ui->displayName->setText(appName);

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
        ui->flag->setText("新增");
    }
}


void MainWindow::on_sign_record_clicked()
{
    DialogSignRecord *sr=new DialogSignRecord(this);
    sr->sn=this->sn;
    sr->setWindowTitle("签名记录");
    sr->show();
}

void MainWindow::execPrint(QString content){
    this->ui->execResult->appendPlainText(content);
}
//void MainWindow::on_fenfa_platform_clicked()
//{
//    QWebEngineView *webView = new QWebEngineView;
//    webView->setWindowTitle("飞鹰分发");
//    QUrl url=QUrl::fromUserInput("http://www.eagleff.top");
//    webView->setUrl(url);
//    QDesktopWidget * desktopWidget = QApplication::desktop();
//    int width=desktopWidget->availableGeometry().width();
//    int height=desktopWidget->availableGeometry().height();
//    webView->setMinimumHeight(height);
//    webView->setMinimumWidth(width);
//    webView->show();
//}


void MainWindow::on_batchRsignButton_clicked()
{
    BatchRSign *batchRSign = new BatchRSign(this);
    batchRSign->setWindowTitle("批量签名");
    batchRSign->show();
}


void MainWindow::uiReset(){
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
    ui->flag->setText("");
    ui->bundleId->setText("");
}

void MainWindow::on_ccNames_currentIndexChanged(const QString &arg1)
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

void MainWindow::on_batchUpdateRsignButton_clicked()
{
    BatchUpdate *batchUpdate = new BatchUpdate(this);
    batchUpdate->setWindowTitle("批量更新");
    batchUpdate->show();
}

void MainWindow::on_isPushMobileProvision_stateChanged(int arg1)
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

void MainWindow::on_supplement_sign_button_clicked()
{
    SupplementSign *supplementSign = new SupplementSign(this);
    supplementSign->show();
}

void MainWindow::on_batch_supplement_sign_button_clicked()
{
    BatchSupplementSign *supplementSign = new BatchSupplementSign(this);
    supplementSign->show();
}

void MainWindow::on_setExpaire_stateChanged(int arg1)
{
    qDebug() << arg1;
    if(arg1==2){
        ui->warning_message->setText(WARNING_MESSAGE);
    }
}
