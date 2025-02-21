#pragma once

#include <QDebug>
#include <QObject>
#include <QException>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <ramtokenstorage.h>

/*
 * This class is responsible for communication with the server over HTTP
 * (typically, data is retrieved from the server, and one-time data is sent to the server via this protocol).
 */
class HttpClient : public QObject
{
    Q_OBJECT

public:
    HttpClient();
    void setRamTokenStorage(RamTokenStorage *ramTokenStorage);
signals:
    void unauthorized();
    void signProcessed();
    void signError(QString error);
    void shouldConfirmEmail();
private slots:
    void sign(QString endpoint, QMap<QString, QString> requestBody);
private:
    RamTokenStorage *ramTokenStorage;
    QNetworkRequest formRequest(QString endpoint);
    QNetworkRequest formRequest(QString endpoint, QMap<QString, QString> requestBody);
    void setAuthorizationHeader(QNetworkRequest request);
    QString getParamsAsUrlUnencoded(QMap<QString, QString> params);
};
