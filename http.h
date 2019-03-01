#ifndef HTTP_H
#define HTTP_H

#include <QObject>
#include "QString"
#include "QNetworkAccessManager"
#include "QNetworkRequest"
#include <QEventLoop>
#include <QNetworkReply>
#include <QFile>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QByteArray>
#include <QMessageBox>

//const QString HTTP_SERVER="https://isign.yz-ss.top";
const QString HTTP_SERVER="http://127.0.0.1:8081";
//const QString HTTP_SERVER="http://qian.whbtsj.com";

class Http:public QObject
{
    Q_OBJECT
public:
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QFile *file;
    Http(QObject *parent);
    QString get(QString url);
    void getFileDownload(QString fileUrl,QString downFilePath);

private slots:
    void readContent();
    void replyFinished(QNetworkReply*);
    void loadError(QNetworkReply::NetworkError);
};

#endif // HTTP_H
