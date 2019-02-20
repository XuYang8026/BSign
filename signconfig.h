#ifndef SIGNCONFIG_H
#define SIGNCONFIG_H

#include <QString>

class SignConfig
{
public:
    SignConfig();

    QString bundleId;

    QString displayName;

    bool useMobileProvsion = false;

    bool signNib = false;

    bool signFramwork = false;

    bool signDylib = false;

    QString expireTime;

    QString ccName;
};

#endif // SIGNCONFIG_H
