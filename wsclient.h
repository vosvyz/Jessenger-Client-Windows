#pragma once

#include <QDebug>
#include <ramtokenstorage.h>

class WsClient : public QObject
{
    Q_OBJECT

public:
    WsClient();
    void setRamTokenStorage(RamTokenStorage *ramTokenStorage);
private:
    RamTokenStorage *ramTokenStorage;
};
