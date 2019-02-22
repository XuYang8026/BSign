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

typedef void (*CallBackPrint) (QString info);

class SignUtil : public QObject
{
    Q_OBJECT
public:
    SignUtil();

    QString mobileProvisionPath;
    IpaInfo *ipaInfo;
    void readIpaInfo(QString filePath);
    bool sign(IpaInfo *ipaInfo,SignConfig *signConfig,CallBackPrint cbp);
    QString findSpecialFileQprocessParamsHandle(QString params,QString param);
};

#endif // SIGNUTIL_H
