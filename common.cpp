#include "common.h"

QString desktopPath="";
QString workspacePath="";

Common::Common()
{

}

//读取本地电脑安装的证书
QStringList Common::readCert(){
    QProcess *process = new QProcess;
    QStringList shellOptions;
    shellOptions << "-c";
    shellOptions << "security find-identity -v | awk 'NR!=1{print p}{p=$0}' | awk -F '\"' '{print $2}'";
    process->start("/bin/bash",shellOptions);
    process->waitForFinished();
    QString result = process->readAllStandardOutput();
    process->close();
    QStringList ccNamesList=result.split("\n");
    ccNamesList.removeAt(ccNamesList.size()-1);
    return ccNamesList;
}
//读取本地电脑序列号
QString Common::readSN(){
    QStringList snParams;
    snParams << "-c";
    snParams << "ioreg -rd1 -c IOPlatformExpertDevice |  awk '/IOPlatformSerialNumber/ { print $3; }'";
    QProcess *p = new QProcess;
    p->start("/bin/bash",snParams);
    p->waitForFinished();
    QString sn=p->readAllStandardOutput().trimmed().replace("\"","");
    return sn;
}

QString Common::execShell(QString cmd){
    QProcess *process = new QProcess;
    QStringList shellOptions;
    shellOptions << "-c";
    shellOptions << cmd;
    process->start("/bin/bash",shellOptions);
    process->waitForFinished();
    QString result = process->readAllStandardOutput();
    process->close();
    return result.replace("\n","");
}

QString Common::getMobileProvisionPath(QString cnName,bool isPush){
    qDebug() << "工作空间："+workspacePath;
    QFileInfoList fileInfoList=GetFileList(workspacePath+"/"+cnName);
    if(fileInfoList.size()<1){
        return "";
    }
    QString filePath;
    for(QFileInfo fileInfo:fileInfoList){
        if(isPush==1){
            QString baseName=fileInfo.baseName();
            QString pushFlag=baseName.mid(0,baseName.size()-5);
            if(pushFlag=="_push"){
                filePath = fileInfo.filePath();
                continue;
            }
        }else{
            filePath = fileInfo.filePath();
        }
    }
    return filePath;
}

