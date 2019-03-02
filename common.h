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

const QString libisigntoolhookFilePath = "/tmp/libisigntoolhook.dylib";
const QString optoolFilePath = "/tmp/optool";
const QString libisigntoolappcountFilePath="/tmp/libisigntoolappcount.dylib";

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
};

#endif // COMMON_H
