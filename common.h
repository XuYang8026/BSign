#ifndef COMMON_H
#define COMMON_H

#include "QString"
#include "QStringList"
#include <QProcess>
#include "QDebug"



class Common
{
public:
    Common();

    static QString readSN();

    static QStringList readCert();
};

#endif // COMMON_H
