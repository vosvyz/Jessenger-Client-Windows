#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QException>
#include <QNetworkRequest>
#include <ramtokenstorage.h>

// This is the super class for the HttpClient and WsClient classes, which interact with the server. It implements some authorization-related functionality required by both classes.
class NetworkClient : public QObject
{
    Q_OBJECT
public:
    void setRamTokenStorage(RamTokenStorage *ramTokenStorage);
signals:
    void unauthorized();
protected:
    RamTokenStorage *ramTokenStorage;
    QNetworkRequest formHttpRequest(QString endpoint);
    QNetworkRequest formHttpRequest(QString endpoint, QMap<QString, QString> body);
    QNetworkRequest formWsRequest(QString endpoint);
    void setAuthorizationHeader(QNetworkRequest &request);
    void refreshToken(QMap<QString, QString> body);
    QByteArray getParamsAsUrlUnencoded(QMap<QString, QString> params);
};

#endif // NETWORKCLIENT_H
