#ifndef ITHREAD_H
#define ITHREAD_H

#include "QString"
#include "QThread"
#include "QObject"
#include "QDateTime"
#include "QDir"
#include "QFileInfo"
#include "QFile"
#include "QProcess"
#include "QDebug"
#include "QStringList"
#include "ipainfo.h"


class IThread:public QThread
{
    Q_OBJECT
public:
    IThread();
    QString filePath;
    void run();
signals:
    void send(IpaInfo *ipaInfo);
    void execPrint(QString execInfo);
};

#endif // ITHREAD_H
