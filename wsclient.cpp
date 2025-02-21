#include "wsclient.h"

WsClient::WsClient() {} // Required by QObject

void WsClient::setRamTokenStorage(RamTokenStorage *ramTokenStorage)
{
    this->ramTokenStorage = ramTokenStorage;
}
