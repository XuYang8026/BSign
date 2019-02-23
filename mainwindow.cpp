#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ifile.h"
#include "common.h"
#include "QDesktopWidget"

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
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    this->setWindowTitle("欢迎使用 IPA Rsign");
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
    QProcess *process = new QProcess;
    QStringList shellOptions;
    shellOptions << "-c";
    shellOptions << "security find-identity -v | awk 'NR!=1{print p}{p=$0}' | awk -F '\"' '{print $2}'";
    process->start("/bin/bash",shellOptions);
    process->waitForFinished();
    QString result = process->readAllStandardOutput();
    qDebug() << result;
    this->ccNames=result.split("\n");
    ui->ccNames->addItems(this->ccNames);

    this->sn=readSN();
    qDebug() << "mac 序列号 ==> "+sn;
    connect(ui->signButton, SIGNAL(clicked()), this,SLOT(signIpa()));
    this->initial();
    this->validate();

    QLabel *locationLabel = new QLabel("序列号："+sn+"    有效期："+this->expireTime+"   签名以及租售企业证书账号    QQ:2449450354");
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
    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), desktopPath,  tr("ipa(*.ipa)"));
    qDebug() << "选择文件路径："+filePath;
    ui->filePath->setText(filePath.trimmed());

    if(filePath.trimmed()!=""){
        IThread *ithread = new IThread;
        ithread->filePath=filePath.trimmed();
        connect(ithread,SIGNAL(send(IpaInfo*)),this,SLOT(setIpaInfo(IpaInfo*)));
        ithread->start();

    }
}


void MainWindow::on_provisionButton_clicked()
{

    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), "",  tr("file(*.mobileprovision)"));
    qDebug() << "选择文件路径："+filePath;
    this->mobileProvisionPath=filePath;
    ui->provisionFilePath->setText(filePath);
}

void MainWindow::readProcessData(){
    ui->execResult->appendPlainText(process->readAllStandardOutput());
}

void MainWindow::signIpa(){

    if(ui->filePath->text().isEmpty()||mobileProvisionPath.isEmpty()||ui->ccNames->currentText()=="请选择证书"||ui->ccNames->currentText().isEmpty()){
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
    signConfig->mobileProvisionPath=mobileProvisionPath;
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
    int expireTimeStamp=ui->expaire->dateTime().toTime_t();
    QString url;
    if(ui->setExpaire->isChecked()){
        url=HTTP_SERVER+"/appSign?uuid="+signConfig->ccUuid+"&bundleId="+bundleId+"&warningMessage="+warningMessage+"&expireTime="+QString::number(expireTimeStamp,10)+"&device="+this->sn+"&ccName="+ui->ccNames->currentText()+"&appName="+deployAppName;
    }else{
        url=HTTP_SERVER+"/appSign?uuid="+signConfig->ccUuid+"&bundleId="+bundleId+"&device="+this->sn+"&ccName="+ui->ccNames->currentText()+"&appName="+deployAppName;
    }
    Http *http = new Http(NULL);
    qDebug() << "请求url："+url;
    QString result=http->get(url);
    if(result!="true"){
        QMessageBox::about(NULL, tr(""),"签名失败，请重新尝试");
        return;
    }
}

void MainWindow::validate(){
        //    获取当前时间
        Http *iHttp = new Http(NULL);
        QString respData = iHttp->get(HTTP_SERVER+"/deviceInfo?device="+this->sn).trimmed();

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
            QMessageBox::warning(this, tr("QMessageBox::information()"),"软件已过期\n请联系QQ:2449450354");
            exit(0);
        }
}


void MainWindow::on_clearLog_clicked()
{
    ui->execResult->setPlainText("");
}

void MainWindow::setIpaInfo(IpaInfo *ipaInfo){
    this->ui->bundleId->setText(ipaInfo->bundleId);
    this->ui->displayName->setText(ipaInfo->deployAppName);
    this->ipaInfo=ipaInfo;
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

