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
#include <QJsonArray>

const QString QQ="3469427753";
extern QString optoolFilePath;
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

    static bool deleteDirectory(const QString &path);
};

#endif // COMMON_H
