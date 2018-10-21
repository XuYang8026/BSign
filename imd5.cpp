#include "imd5.h"

imd5::imd5()
{

}

QString imd5::encode(QString data){
    return this->encode(data,"");
}
QString imd5::encode(QString data,QString salt){
    QString md5;
    QByteArray ba,bb;
    ba.append(data);
    ba.append(salt);
    QCryptographicHash md(QCryptographicHash::Md5);
    md.addData(ba);
    bb = md.result();
    md5.append(bb.toHex());
    return md5;
}
