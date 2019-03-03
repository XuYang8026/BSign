#ifndef APPSIGN_H
#define APPSIGN_H

#include <QString>

class AppSign
{
public:
    AppSign();
    QString device;
    QString uuid;
    QString bundleId;
    QString expireTime;
    QString warningMessage;
    QString createTime;
    int id;
    QString ccName;
    QString appName;
    QString remarks;
    int isPush;
    QString connectInfo;
    QString specialInfo;
    QString updateTime;


};

#endif // APPSIGN_H
