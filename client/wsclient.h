#pragma once

#include <QDebug>
#include <QWebSocket>
#include <QTimer>
#include <networkclient.h>
#include <ramtokenstorage.h>

class WsClient : public NetworkClient
{
    Q_OBJECT

public:
    WsClient();
    ~WsClient();
signals:
    void socketConnected();
    void socketDisconnected();
    void socketMessageReceived(QJsonObject data);
public slots:
    void initialize();
    void sendMessage(QJsonObject messageData);
private slots:
    void messageReceived(QString message);
    void socketStateChanged(QAbstractSocket::SocketState state);
    void resendPendingMessages();
private:
    QAbstractSocket::SocketState socketState;
    void connect();
    void connectSignals();
    QWebSocket *socket;
    QTimer *resendPendingMessagesTimer;
    QVector<QJsonObject> pendingMessages;
};
