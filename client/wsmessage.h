#pragma once

#include <QDebug>

// This class is responsible for storing temporary message data needed for attempting to resend the message in case of failure.
class WsMessage {
public:
    WsMessage(QJsonObject content);
    QJsonObject getContent();
private:
    QJsonObject content;
};
