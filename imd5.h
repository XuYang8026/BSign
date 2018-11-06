#ifndef IMD5_H
#define IMD5_H

#include "QString"
#include "QCryptographicHash"

const QString salt = "44a160d3f98c8a913ca192c7a6222790";

class imd5
{
public:
    imd5();
    QString encode(QString data);
    QString encode(QString data,QString salt);
};

#endif // IMD5_H
