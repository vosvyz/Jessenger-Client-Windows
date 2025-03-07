#pragma once

#include <QDebug>
#include <QString>
#include <QFile>
#include <QDir>

// This class is responsible for storing refresh token in the file system.
class PersistentTokenStorage {
public:
    PersistentTokenStorage();
    QString readRefreshToken();
    void persistRefreshToken(QString refreshToken);
private:
    QFile refreshTokenFile;
};
