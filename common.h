#ifndef COMMON_H
#define COMMON_H

#include "QString"
#include "QStringList"
#include <QProcess>
#include "QDebug"
#include "QDir"

const QString libisigntoolhookFilePath = "/tmp/libisigntoolhook.dylib";
const QString optoolFilePath = "/tmp/optool";
const QString libisigntoolappcountFilePath="/tmp/libisigntoolappcount.dylib";


class Common
{
public:
    Common();

    static QString readSN();

    static QStringList readCert();
};

#endif // COMMON_H
