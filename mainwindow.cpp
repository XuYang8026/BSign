#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    //清空工作空间中sign文件夹的内容
    const QString removePath=workspacePath+"/sign";
    Common::deleteDirectory(removePath);

    QDir signWorkSpace(workspacePath+"/sign");
    if(!signWorkSpace.exists()){
        signWorkSpace.mkdir(workspacePath+"/sign");
    }

    bool optoolCopy=QFile::copy(":/optool",optoolFilePath);

    if(!optoolCopy){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未获取重要组件");
    }
    //初始化工作空间
    QDir workspaceDir(workspacePath);
    if(!workspaceDir.exists()){
        workspaceDir.mkdir(workspacePath);
    }
    for(QString ccName:this->ccNames){
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
    this->setWindowTitle("欢迎使用 BSign 签名工具");
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
    qDebug() << desktopPath;

    this->ccNames=Common::readCert();
    ui->ccNames->addItems(this->ccNames);
    connect(ui->signButton, SIGNAL(clicked()), this,SLOT(signIpa()));
    this->initial();
    ui->signButton->setText("立即签名（QQ:"+QQ+"）");
    this->setAcceptDrops(true);
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
        connect(ithread,SIGNAL(execPrint(QString)),this,SLOT(execPrint(QString)));
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

    if(ui->filePath->toPlainText().isEmpty()||ui->provisionFilePath->toPlainText().isEmpty()||ui->ccNames->currentText()=="请选择证书"||ui->ccNames->currentText().isEmpty()){
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
    signConfig->mobileProvisionPath=ui->provisionFilePath->toPlainText();
    signConfig->displayName=ui->displayName->text();
    signConfig->ccName=ui->ccNames->currentText();

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
}

void MainWindow::setIpaInfo(IpaInfo *ipaInfo){
    this->ui->bundleId->setText(ipaInfo->bundleId);
    this->ui->displayName->setText(ipaInfo->deployAppName);
    this->ipaInfo=ipaInfo;
    ui->thirdFileList->clear();
    ui->thirdFileList->addItems(ipaInfo->thirdInjectionInfoList);
}

void MainWindow::execPrint(QString content){
    this->ui->execResult->appendPlainText(content);
}

void MainWindow::uiReset(){
    ui->provisionFilePath->setText("");
    ui->isPushMobileProvision->setChecked(false);
    ui->ccNames->setCurrentText("请选择证书");
    ui->useBundleId->setChecked(false);
    ui->displayName->setText("");
    ui->bundleId->setText("");
    ui->injectionFilePaths->setText("");
}

void MainWindow::on_ccNames_currentIndexChanged(const QString &arg1)
{
    if(ui->ccNames->currentText()=="请选择证书"){
        return;
    }

    mobileProvisionPath=Common::getMobileProvisionPath(arg1,ui->isPushMobileProvision->isChecked());
    if(mobileProvisionPath.isEmpty()){
        QString path=arg1;
        path=path.replace(QRegExp(":"), "/");
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未读取到"+arg1+"相关描述文件\n请将描述文件复制到"+workspacePath+"/"+path+"目录下或手动选择");
    }
    ui->provisionFilePath->setText(mobileProvisionPath);
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

void MainWindow::on_thirdFileList_currentIndexChanged(const QString &arg1)
{

}

void MainWindow::on_thirdFileList_currentIndexChanged(int index)
{
    QString text=ui->thirdFileList->itemText(index);
    qDebug() << text;

    if(text=="第三方动态库卸载" || text.isEmpty() || text.indexOf("已卸载")>=0){
        return;
    }

    QString machOFilePath=ipaInfo->tmpPath+"Payload/"+ipaInfo->appName+"/"+ipaInfo->machOFileName;
    SignUtil::uninstallThirdInjection(machOFilePath,text);
    ui->thirdFileList->setItemText(index,text+" 已卸载");
}
//当用户拖动文件到窗口部件上时候，就会触发dragEnterEvent事件
void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasFormat("text/uri-list")) //只能打开文本文件
        e->acceptProposedAction(); //可以在这个窗口部件上拖放对象
}

void MainWindow::dropEvent(QDropEvent *e){
    QList<QUrl> urls = e->mimeData()->urls();
    if(urls.isEmpty())
        return ;
    QString filePath = urls.first().toLocalFile();
    QFileInfo fileInfo(filePath);
    QString fileSuffix=fileInfo.suffix();
    if(fileSuffix!="ipa"&&fileSuffix!="IPA"&&fileSuffix!="mobileprovision"){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"请选择ipa或mobileprovision文件");
        return;
    }

    if(fileSuffix=="ipa"||fileSuffix=="IPA"){
        ui->filePath->setText(filePath);
        uiReset();
        if(filePath.trimmed()!=""){
            IThread *ithread = new IThread;
            ithread->filePath=filePath;
            connect(ithread,SIGNAL(send(IpaInfo*)),this,SLOT(setIpaInfo(IpaInfo*)));
            connect(ithread,SIGNAL(execPrint(QString)),this,SLOT(execPrint(QString)));
            ithread->start();
        }
    }

    if(fileSuffix=="mobileprovision"){
        ui->provisionFilePath->setText(filePath);
    }

}

void MainWindow::mousePressEvent(QMouseEvent *event){
    qDebug()<<"鼠标 press";
}
void MainWindow::mouseMoveEvent(QMouseEvent *event){
    qDebug()<<"鼠标 move";
}

void MainWindow::on_injectionPositionButton_clicked()
{

    if(this->ipaInfo==NULL){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"请先选择IPA");
        return;
    }

    QString appPath=this->ipaInfo->appPath;

    if(appPath.isNull()||appPath.isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"请先选择IPA");
        return;
    }

    QString cmd="mkdir "+ipaInfo->appPath+"/bsignlib";
    Common::execShell(cmd);

    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setWindowTitle(tr("打开文件"));
    //设置默认文件路径
    fileDialog->setDirectory(appPath+"/Info.plist");
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);

    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    //打印所有选择的文件的路径
    QStringList fileNames;
    if (fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();
    }
    if(fileNames.isEmpty()){
        return;
    }
    QString position=fileNames.at(0);
    if(!position.contains(ipaInfo->appPath)){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"无效路径，请重新选择");
        return;
    }
    int prefixLength=ipaInfo->appPath.length();
    position=position.mid(prefixLength+1);
    if(position.contains("_CodeSignature")){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"不能选择_CodeSignature文件夹，请重新选择");
        return;
    }
    this->injectionPositionPath=position;
}

void MainWindow::on_injection_clicked()
{
    QString position=this->injectionPositionPath;
    QString injectionFilePaths=ui->injectionFilePaths->toPlainText();
    if(injectionFilePaths.isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"未找到有效路径");
        return;
    }
    QStringList paths=injectionFilePaths.split("\n");
    QStringList realPaths;
    for(QString path:paths){
        QString realPath=path.mid(7);
        if(realPath.isEmpty()){
            break;
        }
        QFile file(realPath);
        if(!file.exists()){
            QMessageBox::warning(this, tr("QMessageBox::information()"),"文件路径有误,请删除重新拽入");
            return;
        }
        realPaths.append(realPath);
    }
    QString ccName=ui->ccNames->currentText();
    QString ipaPath=ui->filePath->toPlainText();
    if(ipaPath.isEmpty()){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"请选择IPA文件");
        return;
    }
    if(ccName=="请选择证书"){
        QMessageBox::warning(this, tr("QMessageBox::information()"),"请选择证书");
        return;
    }
    for(QString path:realPaths){
        QFile::copy(path,ipaInfo->appPath+"/"+position+"/");
        QString cmd="cp \""+path+"\" \""+ipaInfo->appPath+"/"+position+"/\"";
        int flag=system(cmd.toLocal8Bit().data());
        qDebug()  << flag;
        SignUtil signUtil;
        QString machOFilePath=ipaInfo->appPath+"/"+ipaInfo->machOFileName;
        QFileInfo dylibInfo(path);
        QString dylibName=dylibInfo.fileName();
        signUtil.dylibInjectionForAppRelativePath(position+"/"+dylibName,ipaInfo->appPath,machOFilePath,ccName);

    }
    QMessageBox::information(this, tr("QMessageBox::information()"),"注入成功");
}

