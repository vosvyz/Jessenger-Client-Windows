#pragma once

#include <QDebug>
#include <QObject>
#include <QException>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <networkclient.h>
#include <QNetworkAccessManager>
#include <ramtokenstorage.h>

/*
 * This class is responsible for communication with the server over HTTP
 * (typically, data is retrieved from the server, and one-time data is sent to the server via this protocol).
 */
class HttpClient : public NetworkClient
{
    Q_OBJECT

public:
    HttpClient();
signals:
    void checkRefreshTokenFailed();
    void refreshTokenVerified();
    void signProcessed();
    void signError(QString error);
    void shouldConfirmEmail();
    void getUserChatsFailed();
    void getUserChatsProcessed(QJsonArray data);
    void findChatsFailed();
    void findChatsProcessed(QJsonArray data);
public slots:
    void checkRefreshToken();
    void getUserChatsProxy();
    void findChatsProxy(QMap<QString, QString> body);
    void sign(QString endpoint, QMap<QString, QString> requestBody);
private:
    void getUserChats(int failCounter);
    void findChats(QMap<QString, QString> body, int failCounter);
    QNetworkRequest formRequest(QString endpoint);
};
