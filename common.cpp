#include "common.h"
#include <QStandardPaths>

QString desktopPath=QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);;
QString workspacePath=desktopPath+"/bsign_workspace";

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
    QStringList ccNames;
    for(QString ccName:ccNamesList){
        QString cName=ccName.mid(0,20);
        if(cName =="iPhone Distribution:"){
            ccNames.append(ccName);
        }
    }
    return ccNames;
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
    QFileInfoList fileInfoList=GetFileList(workspacePath+"/"+cnName.mid(21));
    if(fileInfoList.size()<1){
        return "";
    }
    QString filePath;
    for(QFileInfo fileInfo:fileInfoList){
        QString baseName=fileInfo.baseName();
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

AppSign Common::getAppSign(QString bundleId){
    Http *http = new Http(NULL);
    QJsonObject jsonObject;
    jsonObject.insert("bundleId",bundleId);
    jsonObject.insert("device",readSN());
    QString respBody=http->post(HTTP_SERVER+"/appSign/search",jsonObject);
    QJsonParseError jsonError;
    QJsonDocument parseDoc = QJsonDocument::fromJson(respBody.toLocal8Bit(),&jsonError);
    AppSign appSign;
    appSign.id=0;
    if(parseDoc.isArray()){
        QJsonArray jsonArray=parseDoc.array();

        if(jsonArray.size()==0){
            return appSign;
        }

        QJsonObject jsonObject=jsonArray.at(0).toObject();
        appSign.id = jsonObject["Id"].toInt();
        appSign.device =jsonObject["Device"].toString();
        appSign.uuid = jsonObject["Uuid"].toString();
        appSign.ccName = jsonObject["CcName"].toString();
        appSign.appName = jsonObject["AppName"].toString();
        appSign.bundleId = jsonObject["BundleId"].toString();
        QString expireTime=jsonObject["ExpireTime"].toString();
        appSign.expireTime = expireTime.isEmpty()||expireTime=="0001-01-01 00:00:00"?"":expireTime;
        appSign.warningMessage = jsonObject["WarningMessage"].toString();
        appSign.remarks = jsonObject["Remarks"].toString();
        appSign.createTime =jsonObject["CreateTime"].toString();
        appSign.isPush = jsonObject["IsPush"].toInt();
        appSign.connectInfo = jsonObject["ConnectInfo"].toString();
        appSign.specialInfo = jsonObject["SpecialInfo"].toString();
        appSign.updateTime = jsonObject["UpdateTime"].toString();
    }
    return appSign;
}

