#include "networkclient.h"

void NetworkClient::setRamTokenStorage(RamTokenStorage *ramTokenStorage) {
    this->ramTokenStorage = ramTokenStorage;
}

QNetworkRequest NetworkClient::formHttpRequest(QString endpoint)
{
    QString requestPath = "http://localhost:8080/" + endpoint;
    QUrl url(requestPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return request;
}

QNetworkRequest NetworkClient::formHttpRequest(QString endpoint, QMap<QString, QString> body)
{
    QString requestPath = "http://localhost:8080/" + endpoint + "?" + getParamsAsUrlUnencoded(body);
    QUrl url(requestPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return request;
}

QNetworkRequest NetworkClient::formWsRequest(QString endpoint)
{
    QString requestPath = "ws://localhost:8080/" + endpoint;
    QUrl url(requestPath);
    QNetworkRequest request(url);
    return request;
}

QByteArray NetworkClient::getParamsAsUrlUnencoded(QMap<QString, QString> params)
{
    QByteArray result;
    for (QMap<QString, QString>::iterator iter = params.begin(); iter != params.end(); ++iter) {
        QString key = iter.key();
        QString value = iter.value();
        result.append(QString(key + '=' + value + '&').toUtf8());
    }
    return result;
}

void NetworkClient::setAuthorizationHeader(QNetworkRequest &request)
{
    QMap<QString, QString> body;
    body.insert("refresh", ramTokenStorage->getRefreshToken());
    refreshToken(body);
    QString accessToken = ramTokenStorage->getAccessToken();
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
}

void NetworkClient::refreshToken(QMap<QString, QString> body) {
    if (body["refresh"].isEmpty()) {
        throw std::runtime_error("Wrong or exp");
    }
    QNetworkAccessManager *networkManager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl("http://localhost:8080/sign/refresh"));
    QNetworkReply *reply = networkManager->post(request, getParamsAsUrlUnencoded(body));
    QObject::connect(reply, &QNetworkReply::finished, this, [reply, networkManager, this]() {
        if (reply->error() == QNetworkReply::HostNotFoundError || reply->error() == QNetworkReply::ConnectionRefusedError) {
            throw std::runtime_error("Has no internet connection");
        }
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 401) {
            throw std::runtime_error("Wrong or exp");
        }
        QByteArray dataAsArray = reply->readAll();
        QJsonDocument dataAsDocument = QJsonDocument::fromJson(dataAsArray);
        QJsonObject data = dataAsDocument.object();
        ramTokenStorage->setAccessToken(data["access"].toString());
        reply->deleteLater();
        networkManager->deleteLater();
    });
}
