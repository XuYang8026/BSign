#ifndef SIGNUTIL_H
#define SIGNUTIL_H

#include "ipainfo.h"
#include "QString"
#include "QObject"
#include "QDateTime"
#include "QDir"
#include "QFileInfo"
#include "QFile"
#include "QProcess"
#include "QDebug"
#include "QStringList"
#include "signconfig.h"

class SignUtil
{
public:
    explicit SignUtil();

    QString mobileProvisionPath;
    IpaInfo *ipaInfo;
    void readIpaInfo(QString filePath);
    bool sign(IpaInfo *ipaInfo,SignConfig *signConfig);

signals:
    void signPrint(QString info);
};

#endif // SIGNUTIL_H
