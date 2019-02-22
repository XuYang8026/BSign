#include "signutil.h"

QString SignUtil::findSpecialFileQprocessParamsHandle(QString params,QString param){
    if(params.isEmpty()){
        params="-name \""+param+"\" ";
    }else{
        params+="-o -name \""+param+"\" ";
    }
    return params;
}


void SignUtil::readIpaInfo(QString filePath){
    QDateTime time = QDateTime::currentDateTime();   //获取当前时间
    int timeT = time.toTime_t();
    QString tmp = "/tmp/"+QString::number(timeT,10)+"/";
    QDir tmpDir(tmp);
    if(!tmpDir.exists()){
        tmpDir.mkdir(tmp);
    }
    QFileInfo fileInfo(filePath);
//    //获取ipa文件名
    QString ipaName=fileInfo.fileName();
    QFile::copy(filePath,tmp+ipaName);
    QString zipFile = ipaName.split(".")[0]+".zip";
    QFile::rename(tmp+ipaName,tmp+zipFile);

    QString cmd = "unzip \""+tmp+zipFile+"\" -d "+tmp;
    system(cmd.toLocal8Bit().data());

    QDir dir(tmp);
    QFileInfoList file_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for(QFileInfo fileInfo:file_list){
        QString floderName=fileInfo.fileName();
        QString cmd = "mv \""+tmp+floderName+"\" "+tmp+"Payload";
        system(cmd.toLocal8Bit().data());
    }

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
//    system(("rm -rf "+tmp).toLocal8Bit().data());
    //读取应用打包名称
    QStringList deployAppNameParams;
    deployAppNameParams << "-c";
    deployAppNameParams << "/usr/libexec/PlistBuddy -c 'Print :CFBundleDisplayName' "+tmp+"Payload/*.app/Info.plist";
    p->start("/bin/bash",deployAppNameParams);
    p->waitForFinished();
    QString deployAppName=p->readAllStandardOutput().trimmed();
    p->close();
    delete p;

    if(deployAppName.isEmpty()){
        deployAppName=appName.split(".")[0];
    }

    IpaInfo *ipaInfo = new IpaInfo;
    ipaInfo->appName=appName;
    ipaInfo->deployAppName=deployAppName;
    ipaInfo->bundleId=bundleId.trimmed();
    ipaInfo->ipaName=ipaName;
    ipaInfo->ipaPath=fileInfo.absolutePath();
    ipaInfo->machOFileName=machOFileName;
    ipaInfo->tmpPath=tmp;
    this->ipaInfo=ipaInfo;
}

bool SignUtil::sign(IpaInfo *ipaInfo,SignConfig *signConfig,CallBackPrint cbp){
    QString tmp=ipaInfo->tmpPath;
    QString appName=ipaInfo->appName;
    QString bundleId=ipaInfo->bundleId;
    QString machOFileName=ipaInfo->machOFileName;
    QString ipaName=ipaInfo->ipaName;
    QString deployAppName=ipaInfo->deployAppName;

    cbp("machOFileName is "+machOFileName);
    //删除原来签名文件
    QString cmd="rm -rf "+tmp+"Payload/"+appName+"/_CodeSignature";
    qDebug() << "执行命令："+cmd;
    int flag = system(cmd.toLocal8Bit().data());
    if(flag!=0){
        cbp("删除原来签名文件失败");
        return false;
    }
    //生成plist文件
    cmd = "/usr/bin/security cms -D -i \""+mobileProvisionPath+"\" > "+tmp+"entitlements_full.plist";
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
    QString uuid = uuidProcess->readAllStandardOutput().trimmed();
    qDebug() << "UUID ====> "+uuid;

    cmd="echo "+uuid+" > "+tmp+"Payload/"+appName+"/uuid";
    qDebug() << "执行命令："+cmd;
    flag = system(cmd.toLocal8Bit().data());
    if(flag!=0){
        cbp("写入uuid文件失败");
        return false;
    }

    cmd="/usr/libexec/PlistBuddy -x -c 'Print:Entitlements' "+tmp+"entitlements_full.plist > "+tmp+"entitlements.plist";
    qDebug() << "执行命令："+cmd;
    flag = system(cmd.toLocal8Bit().data());
    if(flag!=0){
        cbp("生成plist文件失败");
        return false;
    }

    //修改BundleId
    if(signConfig->bundleId!=ipaInfo->bundleId){
        cmd="plutil -replace CFBundleIdentifier -string "+signConfig->bundleId+" "+tmp+"Payload/"+appName+"/info.plist";
//        qDebug() << "执行命令："+cmd;
        flag = system(cmd.toLocal8Bit().data());
        if(flag!=0){
            cbp("修改BundleId失败");
            return false;
        }
    }

    //修改Display Name
    if(signConfig->displayName!=ipaInfo->deployAppName){
        cmd="plutil -replace CFBundleDisplayName -string "+signConfig->displayName+" "+tmp+"Payload/"+appName+"/info.plist";
        qDebug() << "执行命令："+cmd;
        flag = system(cmd.toLocal8Bit().data());
        if(flag!=0){
            cbp("Display Name Update Fail");
            return false;
        }
    }

    //使用证书BundleID
    if(signConfig->useMobileProvsion){

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
            cbp("修改BundleId失败");
            return false;
        }
        cmd="plutil -replace CFBundleIdentifier -string "+mpBundleId+" "+tmp+"Payload/"+appName+"/info.plist";
        qDebug() << "执行命令："+cmd;
        flag = system(cmd.toLocal8Bit().data());
        if(flag!=0){
            cbp("修改BundleId失败");
            return false;
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

    cmd="cp \""+mobileProvisionPath+"\" "+tmp+"Payload/"+appName+"/embedded.mobileprovision";
    flag=system(cmd.toLocal8Bit().data());
    if(flag!=0){
        cbp("复制mobileprovision文件失败");
        return false;
    }

    //特殊文件重签名 start
    QString execParam;

    if(signConfig->signNib){
        execParam=findSpecialFileQprocessParamsHandle(execParam,"*.nib");
    }

    if(signConfig->signFramwork){
        execParam=findSpecialFileQprocessParamsHandle(execParam,"*.framework");
    }

    if(signConfig->signDylib){
        execParam=findSpecialFileQprocessParamsHandle(execParam,"*.dylib");
    }
    if(!execParam.isEmpty()){
        QStringList execParams;
        execParams << "-c";
        execParams << "/usr/bin/find "+tmp+"Payload/"+appName+" "+execParam;
        QProcess *findSpecialFile=new QProcess;
        findSpecialFile->start("/bin/bash",execParams);
        findSpecialFile->waitForFinished();
        QString specialFile = findSpecialFile->readAllStandardOutput().trimmed();
        if(!specialFile.isEmpty()){
            QStringList specialFileList= specialFile.split("\n");
            cbp("重签以下文件：");
            for(QString fi : specialFileList){
                cmd="/usr/bin/codesign --force --sign \""+signConfig->ccName+"\" \""+fi+"\"";
                qDebug() << "重签名命令："+cmd;
                flag=system(cmd.toLocal8Bit().data());
                if(flag!=0){
//                    cbp(fi+"重签名失败");
                    return false;
                }
                QStringList fiList=fi.split("/");
                cbp(fiList[fiList.size()-1]);
            }
        }
    }
    //特殊文件重签名 end

    if (!signConfig->expireTime.isEmpty()){
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
        cmd="cp /tmp/libisigntoolhook.dylib "+tmp+"Payload/"+appName+"/libisigntoolhook.dylib";
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            cbp("植入代码迁移失败");
            return false;
        }
        cmd="chmod +x "+tmp+"Payload/"+appName+"/"+machOFileName;
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            cbp("签名失败！");
            return false;
        }

        //注意 第三方库要单独重签名
        cmd="/usr/bin/codesign --force --sign \""+signConfig->ccName+"\" \""+tmp+"Payload/"+appName+"/libisigntoolhook.dylib"+"\"";
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            cbp("植入代码重签名失败");
            return false;
        }
        cbp("开始注入代码");
        cmd="cd "+tmp+"Payload/"+appName+";"+"/tmp/optool install -c load -p \"@executable_path/libisigntoolhook.dylib\" -t "+machOFileName;
//        cmd="cd "+tmp+"Payload/"+this->appName+";yololib "+machOFileName+" libisigntoolhook.dylib";
        qDebug() << "执行命令："+cmd;
        flag=system(cmd.toLocal8Bit().data());
        if(flag!=0){
            cbp("代码注入失败！");
            return false;
        }
        cbp("代码注入成功");
    }

    cmd="/usr/bin/codesign -f --entitlements \"" + tmp+"entitlements.plist\""+ " -s \"" + signConfig->ccName + "\" " + tmp+"Payload/"+appName;
    qDebug() << "执行命令："+cmd;
    flag=system(cmd.toLocal8Bit().data());
    if(flag!=0){
        cbp("签名失败！");
        return false;
    }

    QString newIPA=ipaName.split(".")[0]+"_resigned.ipa";
    QFile isResigned(ipaInfo->ipaPath+"/"+newIPA);
    if(isResigned.exists()){
        qDebug() << "delete old sign package";
        isResigned.remove();
    }

    cmd="cd "+tmp+";zip -qr ../\""+newIPA+"\" Payload";
    qDebug() << "执行命令："+cmd;
    flag=system(cmd.toLocal8Bit().data());
    if(flag!=0){
        cbp("打包失败！");
        return false;
    }

    cmd="rm -rf "+tmp;
    qDebug() << "执行命令："+cmd;
    system(cmd.toLocal8Bit().data());

    cmd="mv /tmp/\""+newIPA+"\" \""+ipaInfo->ipaPath+"/\"";
    qDebug() << "执行命令："+cmd;
    system(cmd.toLocal8Bit().data());
    cbp("签名完成！新包地址："+ipaInfo->ipaPath+"/"+newIPA);
    return true;
}