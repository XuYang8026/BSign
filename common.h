#ifndef COMMON_H
#define COMMON_H

#include "QString"
#include "QStringList"
#include <QProcess>
#include "QDebug"
#include "QDir"

const QString execProgramPath=QDir().absolutePath();
const QString libisigntoolhookFilePath = execProgramPath+"/libisigntoolhook.dylib";
const QString optoolFilePath = execProgramPath+"/optool";


class Common
{
public:
    Common();

    static QString readSN();

    static QStringList readCert();
};

#endif // COMMON_H
