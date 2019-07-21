#ifndef SIGNUTIL_H
#define SIGNUTIL_H

#include <QObject>
#include "ipainfo.h"
#include "QString"
#include "QDateTime"
#include "QDir"
#include "QFileInfo"
#include "QFile"
#include "QProcess"
#include "QDebug"
#include "QStringList"
#include "signconfig.h"
#include "common.h"

typedef void (*CallBackPrint) (QString info);

class SignUtil:public QObject
{
Q_OBJECT
public:
    explicit SignUtil(QObject *parent = nullptr);

    IpaInfo *ipaInfo;
    //读取ipa信息
    void readIpaInfo(QString filePath);

    bool sign(IpaInfo *ipaInfo,SignConfig *signConfig);
    QString findSpecialFileQprocessParamsHandle(QString params,QString param);
    //注入第三方库
    bool dylibInjection(QString dylibPath,QString machOFilePath,QString ccName);
    //以.app文件作为相对路径注入
    bool dylibInjectionForAppRelativePath(QString relativePath,QString appPath,QString machOFilePath,QString ccName);

    //读取第三方注入信息
    static QStringList readThirdInjection(QString machOFilePath);
    //卸载第三方注入的库
    static bool uninstallThirdInjection(QString machOFilePath,QString path);

signals:
    void execPrint(QString content);
};

#endif // SIGNUTIL_H
