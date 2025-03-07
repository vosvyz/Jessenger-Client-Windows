#include "wsclient.h"

WsClient::WsClient() {

}

void WsClient::initialize() {
    socket = new QWebSocket();
    resendPendingMessagesTimer = new QTimer();
    resendPendingMessagesTimer->setInterval(10000);
    socketState = QAbstractSocket::UnconnectedState;
    connectSignals();
    connect();
}

WsClient::~WsClient() {
    if (socket->isValid()) {
        socket->close();
    }
    else {
        socket->abort();
    }
}

void WsClient::connectSignals() {
    using Connection = std::tuple<QObject*, const char*, QObject*, const char*>;
    QVector<Connection> connections = {
        { socket, SIGNAL(textMessageReceived(QString)), this, SLOT(messageReceived(QString)) },
        { socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)) },
        { resendPendingMessagesTimer, SIGNAL(timeout()), this, SLOT(resendPendingMessages()) }
    };

    for (const auto& conn : connections) {
        QObject::connect(std::get<0>(conn), std::get<1>(conn), std::get<2>(conn), std::get<3>(conn));
    }
}

void WsClient::socketStateChanged(QAbstractSocket::SocketState state) {
    if (state == QAbstractSocket::ConnectedState) {
        resendPendingMessagesTimer->start();
        emit socketConnected();
    }
    else if (state == QAbstractSocket::UnconnectedState){
        resendPendingMessagesTimer->stop();
        emit socketDisconnected();
        connect();
    }
    socketState = state;
}

void WsClient::connect() {
    QNetworkRequest request = formWsRequest("connect/websocket");
    try {
        setAuthorizationHeader(request);
    }
    catch (const std::runtime_error& e) {
        QString what = e.what();
        if (what == "Wrong or exp") {
            emit unauthorized();
        }
        else if (what == "Has no internet connection") {
            return ;
            connect();
        }
        return ;
    }
    socket->open(request);
}

void WsClient::messageReceived(QString message) {
    QByteArray dataAsArray = message.toUtf8();
    QJsonDocument dataAsDocument = QJsonDocument::fromJson(dataAsArray);
    QJsonObject data = dataAsDocument.object();
    if (data["method"] == "acknowledged") { // Acknowledged is set when the server processed the message
        QString tempId = data["tempId"].toString();
        for (int i = 0; i < pendingMessages.size(); ++i) {
            QJsonObject currentMessage = pendingMessages.at(i);
            QString currentMessageTempId = currentMessage["tempId"].toString();
            if (currentMessageTempId == tempId) {
                pendingMessages.remove(i);
                break;
            }
        }
    }
    else {
        emit socketMessageReceived(data);
    }
}

void WsClient::sendMessage(QJsonObject messageData) {
    QDateTime utcDateTime = QDateTime::currentDateTimeUtc();
    qlonglong msecs = utcDateTime.toMSecsSinceEpoch();
    QString tempId = QUuid::createUuid().toString();
    messageData["time"] = QString::number(msecs);
    messageData["tempId"] = tempId;
    pendingMessages.push_back(messageData);
    if (socketState == QAbstractSocket::ConnectedState) {
        socket->sendTextMessage(QJsonDocument(messageData).toJson());
    }
}

void WsClient::resendPendingMessages() {
    QDateTime utcDateTime = QDateTime::currentDateTimeUtc();
    qlonglong msecs = utcDateTime.toMSecsSinceEpoch();
    for (int i = 0; i < pendingMessages.size(); ++i) {
        QJsonObject currentMessage = pendingMessages.at(i);
        qlonglong currentMessageTime = currentMessage["time"].toVariant().toLongLong();
        if (currentMessageTime + 10000 < msecs) {
            if (socketState == QAbstractSocket::ConnectedState) {
                socket->sendTextMessage(QJsonDocument(currentMessage).toJson());
            }
        }
    }
}
