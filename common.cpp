#include "common.h"
#include <QStandardPaths>

QString desktopPath=QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);;
QString workspacePath=desktopPath+"/bsign_workspace";
QString optoolFilePath =workspacePath+"/sign/optool";

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
//    QStringList ccNames;
//    for(QString ccName:ccNamesList){
//        QString cName=ccName.mid(0,20);
//        if(cName =="iPhone Distribution:"){
//            ccNames.append(ccName);
//        }
//    }
//    return ccNames;
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
    qDebug() << "执行shell命令："+cmd;
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
    cnName=cnName.replace(QRegExp("/"), ":");
    QFileInfoList fileInfoList=GetFileList(workspacePath+"/"+cnName);
    if(fileInfoList.size()<1){
        return "";
    }
    QString filePath;
    for(QFileInfo fileInfo:fileInfoList){
        QString baseName=fileInfo.baseName();
        QString suffix=fileInfo.suffix();
        if(suffix!="mobileprovision"){
            continue;
        }
        QString pushFlag=baseName.mid(baseName.size()-5);
        if(isPush){
            if(pushFlag=="_push"){
                filePath = fileInfo.filePath();
                break;
            }
        }else{
            if(pushFlag!="_push"){
                filePath = fileInfo.filePath();
                break;
            }
        }
    }
    return filePath;
}

bool Common::deleteDirectory(const QString &path)
{
    if (path.isEmpty())
        return false;

    QDir dir(path);
    if(!dir.exists())
        return true;

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo fi, fileList)
    {
        if (fi.isFile())
            fi.dir().remove(fi.fileName());
        else
            deleteDirectory(fi.absoluteFilePath());
    }
    return dir.rmpath(dir.absolutePath());
}

