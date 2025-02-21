#pragma once

#include <QDebug>
#include <QString>
#include <QFile>
#include <QDir>

// This class is responsible for storing tokens in the file system.
class PersistentTokenStorage {
public:
    PersistentTokenStorage();
    QString readAccessToken();
    QString readRefreshToken();
    void persistAccessToken(QString accessToken);
    void persistRefreshToken(QString refreshToken);
private:
    QFile accessTokenFile;
    QFile refreshTokenFile;
};
