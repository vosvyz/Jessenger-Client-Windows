#pragma once

#include <QDebug>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <persistenttokenstorage.h>

// This class is responsible for storing authorization tokens in RAM.
class RamTokenStorage {
public:
    RamTokenStorage();
    ~RamTokenStorage();
    QString getAccessToken();
    QString getRefreshToken();
    bool isAccessTokenExpired();
    bool isRefreshTokenExpired();
    void setAccessToken(QString accessToken);
    void setRefreshToken(QString refreshToken);
private:
    QString accessToken;
    QString refreshToken;
    qlonglong getExpiresAt(QString token);
    QString getTokenPayload(QString token);
    PersistentTokenStorage persistentTokenStorage;
};

