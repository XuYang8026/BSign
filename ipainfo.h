#ifndef IPAINFO_H
#define IPAINFO_H

#include "QString"

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
};

#endif // IPAINFO_H
