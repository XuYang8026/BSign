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

void MainWindow::initial(){
    QFile optoolFile(this->optoolPath);
    if(!optoolFile.exists()){
        QMessageBox::critical(NULL, "title", "程序初始化");
        Http *iHttp = new Http(this);
        iHttp->getFileDownload("http://tiger-public.oss-cn-beijing.aliyuncs.com/optool",optoolPath);

    }
    //授予可执行权限
    QString cmd = "chmod +x "+optoolPath;
    int flag=system(cmd.toLocal8Bit().data());
    if (flag!=0){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未获取重要组件执行权限\n尝试重启客户端");
        return;
    }
    //检测libisigntooldylib是否存在
    QDir libisigntooldylibDir(this->libisigntooldylibPath);
    if(!libisigntooldylibDir.exists()){
        Http *iHttp = new Http(this);
        iHttp->getFileDownload("http://public.count321.cn/libisigntoolhook.dylib",this->libisigntooldylibPath);
    }


    ui->expaire->setDateTime(QDateTime::currentDateTime());
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    this->setWindowTitle("欢迎使用Isign-tool");
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
    QString bundleId=ipaInfo->bundleId;
    QString machOFileName=ipaInfo->machOFileName;
    QString ipaName=ipaInfo->ipaName;
    QString deployAppName=ipaInfo->deployAppName;

    ui->execResult->appendPlainText("machOFileName is "+machOFileName);

    //删除原来签名文件
    QString cmd="rm -rf "+tmp+"Payload/"+appName+"/_CodeSignature";
    qDebug() << "执行命令："+cmd;
    int flag = system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("删除原来签名文件失败");
        return;
    }
    //生成plist文件
    cmd = "/usr/bin/security cms -D -i "+mobileProvisionPath+" > "+tmp+"entitlements_full.plist";
    qDebug() << "执行命令："+cmd;
    flag=system(cmd.toLocal8Bit().data());
    qDebug() << flag;

    //读取UUID
    QStringList readUUIDParams;
    readUUIDParams << "-c";
    readUUIDParams << "/usr/libexec/PlistBuddy -c 'Print:UUID' "+tmp+"entitlements_full.plist";
    QProcess *uuidProcess=new QProcess;
    uuidProcess->start("/bin/bash",readUUIDParams);
    uuidProcess->waitForFinished();
    uuid = uuidProcess->readAllStandardOutput().trimmed();
    qDebug() << "UUID ====> "+uuid;

    cmd="echo "+uuid+" > "+tmp+"Payload/"+appName+"/uuid";
    qDebug() << "执行命令："+cmd;
    flag = system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("写入uuid文件失败");
        return;
    }

    cmd="/usr/libexec/PlistBuddy -x -c 'Print:Entitlements' "+tmp+"entitlements_full.plist > "+tmp+"entitlements.plist";
    qDebug() << "执行命令："+cmd;
    flag = system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("生成plist文件失败");
        return;
    }

    //修改BundleId
    if(ui->bundleId->text()!=bundleId && ui->updateBundleIdRadioButton->isChecked()){
        cmd="plutil -replace CFBundleIdentifier -string "+bundleId+" "+tmp+"Payload/"+appName+"/info.plist";
        qDebug() << "执行命令："+cmd;
        flag = system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("修改BundleId失败");
            return;
        }
    }
    //使用证书BundleID
    if(ui->useBundleId->isChecked()){

        QProcess *p = new QProcess;
        QStringList args;
        args.append("-c");
        args.append("/usr/libexec/PlistBuddy "+tmp+"entitlements.plist "+"-c print | grep application-identifier | awk '{print $3}'");
        p->start("/bin/bash",args);
        p->waitForFinished();
        //读取证书BundleID
        QString mpBundleId=p->readAllStandardOutput().trimmed();
        qDebug() << "证书BundleID："+mpBundleId;
        if(mpBundleId.isEmpty()){
            ui->execResult->appendPlainText("修改BundleId失败");
            return;
        }
        cmd="plutil -replace CFBundleIdentifier -string "+mpBundleId+" "+tmp+"Payload/"+appName+"/info.plist";
        qDebug() << "执行命令："+cmd;
        flag = system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("修改BundleId失败");
            return;
        }
    }

    QFile file(tmp+"Payload/"+appName+"/embedded.mobileprovision");
    if(!file.exists()){
        file.open(QIODevice::WriteOnly | QIODevice::Text );
        QTextStream in(&file);
        in << "";
        file.flush();
        file.close();
    }

    cmd="cp "+mobileProvisionPath+" "+tmp+"Payload/"+appName+"/embedded.mobileprovision";
    flag=system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("复制mobileprovision文件失败");
        return;
    }

    QFileInfoList fil=GetFileList(tmp+"Payload/"+appName);

    //NIB 文件重签名
    if(ui->clickNibSign->isChecked()){
        for(QFileInfo fi : fil){
            if(fi.suffix()=="nib"){
                cmd="/usr/bin/codesign --force --sign \""+ui->ccNames->currentText()+"\" \""+fi.absolutePath()+"/"+fi.fileName()+"\"";
                flag=system(cmd.toLocal8Bit().data());
                if(flag!=0){
                    ui->execResult->appendPlainText("nib重签名失败");
                    return;
                }
            }
        }
    }

    if (ui->setExpaire->isChecked()){
        QDir dir(tmp+"Payload");
        QFileInfoList fileInfos = dir.entryInfoList();
        QStringList fileNames;
        foreach(QFileInfo fileinfo, fileInfos)
        {
            QString filter = fileinfo.suffix();    //后缀名
            if(filter != "app"){
                continue;
            }
            fileNames.append(fileinfo.fileName());
        }
        appName=fileNames[0];
        cmd="cp "+this->libisigntooldylibPath+" "+tmp+"Payload/"+appName+"/libisigntoolhook.dylib";
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("植入代码迁移失败");
            return;
        }
        cmd="chmod +x "+tmp+"Payload/"+appName+"/"+machOFileName;
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("签名失败！");
            return;
        }

        //注意 第三方库要单独重签名
        cmd="/usr/bin/codesign --force --sign \""+ui->ccNames->currentText()+"\" \""+tmp+"Payload/"+appName+"/libisigntoolhook.dylib"+"\"";
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("植入代码重签名失败");
            return;
        }
        ui->execResult->appendPlainText("开始注入代码");
        cmd="cd "+tmp+"Payload/"+appName+";"+this->optoolPath+" install -c load -p \"@executable_path/libisigntoolhook.dylib\" -t "+machOFileName;
//        cmd="cd "+tmp+"Payload/"+this->appName+";yololib "+machOFileName+" libisigntoolhook.dylib";
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("代码注入失败！");
            return;
        }
        ui->execResult->appendPlainText("代码注入成功");
    }

    QString ccName=ui->ccNames->currentText();
    cmd="/usr/bin/codesign -f --entitlements \"" + tmp+"entitlements.plist\""+ " -s \"" + ccName + "\" " + tmp+"Payload/"+appName;
    qDebug() << "执行命令："+cmd;
    flag=system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("签名失败！");
        return;
    }

    QString newIPA=ipaName.split(".")[0]+"_resigned.ipa";
    QFile isResigned(this->parentPath+"/"+newIPA);
    if(isResigned.exists()){
        qDebug() << "delete old sign package";
        isResigned.remove();
    }

    cmd="cd "+tmp+";zip -qr ../"+newIPA+" Payload";
    qDebug() << "执行命令："+cmd;
    flag=system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("打包失败！");
        return;
    }

    cmd="rm -rf "+tmp;
    qDebug() << "执行命令："+cmd;
    system(cmd.toLocal8Bit().data());

    cmd="mv /tmp/"+newIPA+" "+ipaInfo->ipaPath+"/";
    qDebug() << "执行命令："+cmd;
    system(cmd.toLocal8Bit().data());

    QString warningMessage=ui->warning_message->text();
    int expireTimeStamp=ui->expaire->dateTime().toTime_t();
    QString url;
    if(ui->setExpaire->isChecked()){
        url=HTTP_SERVER+"/appSign?uuid="+this->uuid+"&bundleId="+bundleId+"&warningMessage="+warningMessage+"&expireTime="+QString::number(expireTimeStamp,10)+"&device="+this->sn+"&ccName="+ui->ccNames->currentText()+"&appName="+deployAppName;
    }else{
        url=HTTP_SERVER+"/appSign?uuid="+this->uuid+"&bundleId="+bundleId+"&device="+this->sn+"&ccName="+ui->ccNames->currentText()+"&appName="+deployAppName;
    }
    Http *http = new Http(NULL);
    qDebug() << "请求url："+url;
    QString result=http->get(url);
    if(result!="true"){
        QMessageBox::about(NULL, tr(""),"签名失败，请重新尝试");
        return;
    }

//    ui->execResult->appendPlainText("签名完成！");
//    QMessageBox::about(NULL, tr(""),"签名完成！新包地址："+ipaInfo->ipaPath+"/"+newIPA);
    ui->execResult->appendPlainText("签名完成！新包地址："+ipaInfo->ipaPath+"/"+newIPA);
    ui->filePath->setText("");
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
            QMessageBox::warning(this, tr("QMessageBox::information()"),"软件已过期\n请联系QQ:3536391351");
            exit(0);
        }
}


void MainWindow::on_clearLog_clicked()
{
    ui->execResult->setPlainText("");
}

void MainWindow::setIpaInfo(IpaInfo *ipaInfo){
    this->ui->bundleId->setText(ipaInfo->bundleId);
    this->ipaInfo=ipaInfo;
}


void MainWindow::on_sign_record_clicked()
{
    DialogSignRecord *sr=new DialogSignRecord(this);
    sr->sn=this->sn;
    sr->setWindowTitle("签名记录");
    sr->show();
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
