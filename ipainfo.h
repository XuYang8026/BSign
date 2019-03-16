#ifndef IPAINFO_H
#define IPAINFO_H

#include "QString"
#include <QStringList>

class IpaInfo
{
public:
    IpaInfo();
    QString machOFileName;
    QString appName;
    QString ipaPath;
    QString ipaName;
    QString tmpPath;
    QString bundleId;
    //应用打包名称
    QString deployAppName;
    //第三方注入列表
    QStringList thirdInjectionInfoList;
};

#endif // IPAINFO_H
