#include "http.h"

Http::Http(QObject *parent)
{
    this->manager = new QNetworkAccessManager(parent);
}
QString Http::get(QString url){
    const QUrl qUrl = QUrl::fromUserInput(url);

    QNetworkRequest request = QNetworkRequest(qUrl);
    QNetworkReply* reply = manager->get(request);
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    QByteArray replyData = reply->readAll();
    reply->deleteLater();
    reply = nullptr;
    return replyData.data();
}
void Http::getFileDownload(QString fileUrl,QString downFilePath){
        this->file=new QFile(downFilePath);
        file->open(QIODevice::WriteOnly);
        this->manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
        QUrl url(fileUrl);

        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
        this->reply = this->manager->get(request);

        connect((QObject *)reply, SIGNAL(readyRead()), this, SLOT(readContent()));
        connect(this->manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(loadError(QNetworkReply::NetworkError)));
}
void Http::readContent()    //下载时向本地文件中写入数据
{
    file->write(reply->readAll());

}
void Http::replyFinished(QNetworkReply*)    //删除指针，更新和关闭文件
{
    if(reply->error() == QNetworkReply::NoError)
        {
            reply->deleteLater();
            file->flush();
            file->close();

        }
        else
        {
            QMessageBox::critical(NULL, tr("Error"), "文件下载失败");

        }
}
void Http::loadError(QNetworkReply::NetworkError)    //传输中的错误输出
{
     qDebug()<<"Error: "<<reply->error();
}

QString Http::post(QString url,QJsonObject jsonObject){
    const QUrl qUrl = QUrl::fromUserInput(url+"?");
    QJsonDocument document;
    document.setObject(jsonObject);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString params="";
    QStringList keys=jsonObject.keys();
    for(QString key:keys){
        params+=key+"="+jsonObject[key].toString()+"&";
    }
    qDebug() << "请求参数"+ params;
    QNetworkRequest request = QNetworkRequest(qUrl);
    QNetworkReply* reply = manager->post(request,params.toUtf8());
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    QByteArray replyData = reply->readAll();
    reply->deleteLater();
    reply = nullptr;
    return replyData.data();
    return "";
}
