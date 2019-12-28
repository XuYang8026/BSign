#ifndef COMMON_H
#define COMMON_H

#include "QString"
#include "QStringList"
#include <QProcess>
#include "QDebug"
#include "QDir"
#include "QDesktopWidget"
#include "QApplication"
#include "ifile.h"
#include "model/appsign.h"
#include "http.h"
#include <QJsonArray>

const QString APP_VERSION="3.0";
const QString WARNING_MESSAGE="签名已到期，请续费后使用";
const QString QQ="3469427753";

extern QString libisigntoolhookFilePath;
extern QString optoolFilePath;
extern QString libisigntoolappcountFilePath;
extern QString desktopPath;
extern QString workspacePath;

class Common
{
public:
    Common();

    static QString readSN();

    static QStringList readCert();

    static QString execShell(QString cmd);
    //通过证书名称获取描述文件路径
    static QString getMobileProvisionPath(QString cnName,bool isPush);
    //通过设备和bundleId获取签名信息
    static AppSign getAppSign(QString bundleId);

    static bool deleteDirectory(const QString &path);
};

#endif // COMMON_H
