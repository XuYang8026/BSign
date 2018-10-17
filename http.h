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
