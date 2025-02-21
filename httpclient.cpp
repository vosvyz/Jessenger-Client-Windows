#include <httpclient.h>

HttpClient::HttpClient() {} // Required by QObject

void HttpClient::setRamTokenStorage(RamTokenStorage *ramTokenStorage)
{
    this->ramTokenStorage = ramTokenStorage;
}

QNetworkRequest HttpClient::formRequest(QString endpoint)
{
    QString requestPath = "http://localhost:8080/" + endpoint;
    QUrl url(requestPath);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return request;
}

QNetworkRequest HttpClient::formRequest(QString endpoint, QMap<QString, QString> requestBody)
{
    QString requestPath = "http://localhost:8080/" + endpoint + "?";
    QString params = getParamsAsUrlUnencoded(requestBody);
    QUrl url(requestPath + params);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return request;
}

void HttpClient::setAuthorizationHeader(QNetworkRequest request)
{
    if (ramTokenStorage->isAccessTokenExpired()) {
        if (ramTokenStorage->isRefreshTokenExpired()) {
            emit unauthorized();
            throw new QException();
        }
    }
}

QString HttpClient::getParamsAsUrlUnencoded(QMap<QString, QString> params)
{
    QString result;
    for (QMap<QString, QString>::iterator iter = params.begin(); iter != params.end(); ++iter) {
        QString key = iter.key();
        QString value = iter.value();
        result.append(key + '=' + value + '&');
    }
    return result;
}

void HttpClient::sign(QString endpoint, QMap<QString, QString> requestBody)
{
    QNetworkAccessManager *networkManager = new QNetworkAccessManager();
    QNetworkRequest request = formRequest(endpoint);
    QNetworkReply *reply = networkManager->post(request, getParamsAsUrlUnencoded(requestBody).toUtf8());
    auto success = QSharedPointer<bool>::create(false);
    auto statusChecked = QSharedPointer<bool>::create(false);
    auto dataAsArray = QSharedPointer<QByteArray>::create(QByteArray());
    QObject::connect(reply, &QNetworkReply::readyRead, this, [success, statusChecked, dataAsArray, reply, this]() {
        if (!*statusChecked) {
            *statusChecked = true;
            QString status = QString::fromUtf8(reply->readAll()).simplified();
            status.replace("data:", "");
            if (status == "Not Found") {
                emit signError("User not found!");
                return;
            }
            else if (status == "Forbidden") {
                emit signError("Wrong password!");
                return;
            }
            else if (status == "Conflict") {
                emit signError("User already exists!");
                return;
            }
            else if (status == "Unprocessable Entity") {
                emit signError("Something went wrong, try again!");
                return;
            }
            else {
                *success = true;
                emit shouldConfirmEmail();
            }
        }
        else {
            QByteArray data = reply->readAll();
            dataAsArray->append(data);
        }
    });
    QObject::connect(reply, &QNetworkReply::finished, this, [success, dataAsArray, reply, networkManager, this]() {
        if (reply->error() == QNetworkReply::HostNotFoundError || reply->error() == QNetworkReply::ConnectionRefusedError) {
            emit signError("We are experiencing some issues on our server! Try again, please.");
        }
        if (*success) {
            QByteArray nonPointerData = *dataAsArray;
            nonPointerData = nonPointerData.replace("data:", "");
            nonPointerData = nonPointerData.simplified();
            QJsonDocument dataAsDocument = QJsonDocument::fromJson(nonPointerData);
            QJsonObject data = dataAsDocument.object();
            ramTokenStorage->setAccessToken(data["access"].toString());
            ramTokenStorage->setRefreshToken(data["refresh"].toString());
            emit signProcessed();
        }
        reply->deleteLater();
        networkManager->deleteLater();
    });
}
