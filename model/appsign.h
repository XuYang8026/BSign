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
    //提示类型 1=弹窗 2=闪退
    int warningType;

};

#endif // APPSIGN_H
