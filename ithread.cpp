#include "ithread.h"

IThread::IThread(){}

void IThread::run(){
    QDateTime time = QDateTime::currentDateTime();   //获取当前时间
    int timeT = time.toTime_t();
    QString tmp = "/tmp/"+QString::number(timeT,10)+"/";
    QDir tmpDir(tmp);
    if(!tmpDir.exists()){
        tmpDir.mkdir(tmp);
    }
    QFileInfo fileInfo(this->filePath);
//    //获取ipa文件名
    QString ipaName=fileInfo.fileName();
    QFile::copy(this->filePath,tmp+ipaName);
    QString zipFile = ipaName.split(".")[0]+".zip";
    QFile::rename(tmp+ipaName,tmp+zipFile);

    QString cmd = "unzip "+tmp+zipFile+" -d "+tmp;
    system(cmd.toLocal8Bit().data());
    QString plistPath=tmp+"Payload/*.app/Info.plist";
    QStringList matchOParams;
    matchOParams << "-c";
    matchOParams << "plutil -convert xml1 -o - "+plistPath+" | grep -A1 Exec|tail -n1 | cut -f2 -d\\> | cut -f1 -d\\<";
    QProcess *p = new QProcess;
    p->start("/bin/bash",matchOParams);
    p->waitForFinished();
    QString machOFileName = p->readAllStandardOutput().trimmed();
    qDebug() << "machOFileName:"+machOFileName;
    QString appName= machOFileName+".app";
    qDebug() << "appName:"+appName;
    QStringList bundleIdParams;
    bundleIdParams << "-c";
    bundleIdParams << "/usr/libexec/PlistBuddy -c \"Print CFBundleIdentifier\" "+plistPath;
    p->start("/bin/bash",bundleIdParams);
    p->waitForFinished();
    QString bundleId = p->readAllStandardOutput();
    delete p;
    system(("rm -rf "+tmp).toLocal8Bit().data());
    emit send(bundleId.trimmed());
    this->wait();
}

