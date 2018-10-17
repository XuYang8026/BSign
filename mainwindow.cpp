#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDateTime"
#include <stdio.h>

void MainWindow::initial(){
    QMessageBox::critical(NULL, "title", "comment");
    bool exist=isFileExist(this->yololibPath);
    if(!exist){
        Http *iHttp = new Http(this);
        iHttp->getFileDownload("http://tiger-public.oss-cn-beijing.aliyuncs.com/yololib",yololibPath);
    }
    QString cmd = "chmod +x "+yololibPath;
    int flag=system(cmd.toLocal8Bit().data());
    if (flag!=0){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未获取重要组件执行权限，请重新启动客户端");
        return;
    }
    exist=isFileExist(this->libisigntooldylibPath);
    qDebug() << exist;
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
    shellOptions << "   security find-identity -v | awk 'NR!=1{print p}{p=$0}' | awk -F '\"' '{print $2}'";
    process->start("/bin/bash",shellOptions);
    process->waitForFinished();
    QString result = process->readAllStandardOutput();
    qDebug() << result;
    this->ccNames=result.split("\n");
    ui->ccNames->addItems(this->ccNames);
    QLabel *locationLabel = new QLabel("author:Jackson      QQ:3536391351");
    locationLabel->setMinimumWidth(640);
    locationLabel->setAlignment(Qt::AlignCenter);
    this->statusBar()->addWidget(locationLabel);
    connect(ui->signButton, SIGNAL(clicked()), this,SLOT(signIpa()));
//    QMessageBox::about(NULL, "About", "About this <font color='red'>application</font>");
    this->initial();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectIpaButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("open file"), desktopPath,  tr("ipa(*.ipa)"));
    qDebug() << "选择文件路径："+filePath;
    ipaPath=filePath;
    ui->filePath->setText(filePath);
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

    if(ipaPath.isEmpty()||mobileProvisionPath.isEmpty()||ui->ccNames->currentText().isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"ipa路径或mobileprovision路径或证书名称不能为空");
        return;
    }
    QDateTime time = QDateTime::currentDateTime();   //获取当前时间
    int timeT = time.toTime_t();
    ui->execResult->appendPlainText("当前时间戳："+QString::number(timeT,10));
    QFileInfo fileInfo(this->ipaPath);
    //获取ipa文件名
    ipaName=fileInfo.fileName();
    ui->execResult->appendPlainText("IPA名称："+ipaName);
    //获取ipa父目录
    parentPath=fileInfo.dir().absolutePath();
    //生成临时目录
    tmp = parentPath+"/"+QString::number(timeT,10)+"/";
    qDebug() << tmp;
    QDir tmpDir(tmp);
    bool exist = tmpDir.exists();
    if(!exist){
        tmpDir.mkpath(tmp);
    }
    //复制ipa
    QFile::copy(ipaPath,tmp+ipaName);
    QFile newIpaFile(tmp+ipaName);
    qDebug() << ipaName.split(".")[0]+".zip";
    QString zipFile = tmp+ipaName.split(".")[0]+".zip";
    QFile::rename(tmp+ipaName,zipFile);
    process->start("unzip "+zipFile+" -d "+tmp);
    connect(process, SIGNAL(readyReadStandardOutput()), this,SLOT(readProcessData()));
    process->waitForFinished();
    QStringList matchOParams;
    matchOParams << "-c";
    matchOParams << "plutil -convert xml1 -o - "+tmp+"Payload/*.app/Info.plist | grep -A1 Exec|tail -n1 | cut -f2 -d\\> | cut -f1 -d\\<";
    QProcess *p = new QProcess;
    p->start("/bin/bash",matchOParams);
    p->waitForFinished();
    this->machOFileName = p->readAllStandardOutput().trimmed();
    delete p;
    ui->execResult->appendPlainText("machOFileName is "+machOFileName);

    //删除原来签名文件
    QString cmd="rm -rf "+tmp+"Payload/*.app/_CodeSignature";
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
    cmd="/usr/libexec/PlistBuddy -x -c 'Print:Entitlements' "+tmp+"entitlements_full.plist > "+tmp+"entitlements.plist";
    qDebug() << "执行命令："+cmd;
    flag = system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("生成plist文件失败");
        return;
    }
    //修改BundleId
    QString bundleId = ui->bundleId->text();
    if(!bundleId.isEmpty()&&ui->updateBundleIdRadioButton->isChecked()){
        cmd="plutil -replace CFBundleIdentifier -string "+bundleId+" "+tmp+"Payload/*.app/info.plist";
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
        cmd="plutil -replace CFBundleIdentifier -string "+mpBundleId+" "+tmp+"Payload/*.app/info.plist";
        qDebug() << "执行命令："+cmd;
        flag = system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("修改BundleId失败");
            return;
        }
    }

    cmd="cp "+mobileProvisionPath+" "+tmp+"Payload/*.app/embedded.mobileprovision";
    flag=system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("复制mobileprovision文件失败");
        return;
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
        this->appName=fileNames[0];
        cmd="mkdir -p "+tmp+"Payload/"+this->appName+"/Frameworks/";
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("签名失败！");
            return;
        }
        cmd="cp "+this->parentPath+"/libisigntoolhook.dylib "+tmp+"Payload/"+this->appName+"/Frameworks/libisigntoolhook.dylib";
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("植入代码迁移失败");
            return;
        }
        cmd="chmod +x "+tmp+"Payload/"+this->appName+"/"+machOFileName;
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("签名失败！");
            return;
        }
//        cmd="/usr/bin/codesign --force --sign \""+ui->ccNames->currentText()+"\" \""+tmp+"Payload/"+this->appName+"/Frameworks/libisigntoolhook.dylib"+"\"";
//        flag=system(cmd.toLocal8Bit().data());
//        if(flag!=0){
//            ui->execResult->appendPlainText("植入代码重签名失败");
//            return;
//        }
    }



    if(ui->setExpaire->isChecked()){
        ui->execResult->appendPlainText("开始注入代码");
        cmd="cd "+tmp+"Payload/"+this->appName+";"+this->yololibPath+" "+machOFileName+" Frameworks/libisigntoolhook.dylib";
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            ui->execResult->appendPlainText("代码注入失败！");
            return;
        }
        ui->execResult->appendPlainText("代码注入成功");
    }

    QString ccName=ui->ccNames->currentText();
    cmd="/usr/bin/codesign -f --entitlements \"" + tmp+"entitlements.plist\""+ " -s \"" + ccName + "\" " + tmp+"Payload/*.app";
    qDebug() << "执行命令："+cmd;
    flag=system(cmd.toLocal8Bit().data());
    if(flag!=0){
        ui->execResult->appendPlainText("签名失败！");
        return;
    }

    QString newIPA=ipaName.split(".")[0]+"_resigned.ipa";
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
    ui->execResult->appendPlainText("签名完成！");
    QMessageBox::about(NULL, tr(""),"签名完成！新包地址："+parentPath+"/"+newIPA);

}


void MainWindow::on_clearLog_clicked()
{
    ui->execResult->setPlainText("");
}
