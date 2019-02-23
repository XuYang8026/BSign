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
    void readIpaInfo(QString filePath);
    bool sign(IpaInfo *ipaInfo,SignConfig *signConfig);
    QString findSpecialFileQprocessParamsHandle(QString params,QString param);
    bool dylibInjection(QString dylibPath,QString machOFilePath,QString ccName);
signals:
    void execPrint(QString content);
};

#endif // SIGNUTIL_H
