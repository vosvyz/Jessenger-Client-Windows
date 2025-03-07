#include <wsmessage.h>

WsMessage::WsMessage(QJsonObject content) {
    this->content = content;
}

QJsonObject WsMessage::getContent() {
    return content;
}
