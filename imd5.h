#ifndef IMD5_H
#define IMD5_H

#include "QString"
#include "QCryptographicHash"

class imd5
{
public:
    imd5();
    QString encode(QString data);
    QString encode(QString data,QString salt);
};

#endif // IMD5_H
