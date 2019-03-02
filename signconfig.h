#ifndef SIGNCONFIG_H
#define SIGNCONFIG_H

#include <QString>

class SignConfig
{
public:
    SignConfig();

    QString mobileProvisionPath;

    QString bundleId;

    QString displayName;

    bool useMobileProvsionBundleId = false;
    //是否是推送证书描述文件
    bool isPushMobileProvsionBundleId = false;

    bool signNib = false;

    bool signFramwork = false;

    bool signDylib = false;

    QString expireTime;

    QString ccName;

    QString warningMessage;
    //证书UUID
    QString ccUuid;

    bool useAppCount = false;
};

#endif // SIGNCONFIG_H
