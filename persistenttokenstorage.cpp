#include <persistenttokenstorage.h>

PersistentTokenStorage::PersistentTokenStorage()
{
    QDir homeDirectory = QDir::home();
    if (!homeDirectory.exists("Jessenger/auth")) {
        homeDirectory.mkdir("Jessenger/auth");
    }
    accessTokenFile.setFileName(homeDirectory.path() + "/Jessenger/auth/accesstoken.txt");
    refreshTokenFile.setFileName(homeDirectory.path() + "/Jessenger/auth/refreshtoken.txt");
}

QString PersistentTokenStorage::readAccessToken()
{
    accessTokenFile.open(QIODevice::ReadOnly);
    QString result = accessTokenFile.readAll();
    accessTokenFile.close();
    return result;
}

QString PersistentTokenStorage::readRefreshToken()
{
    refreshTokenFile.open(QIODevice::ReadOnly);
    QString result = refreshTokenFile.readAll();
    refreshTokenFile.close();
    return result;
}

void PersistentTokenStorage::persistAccessToken(QString accessToken)
{
    accessTokenFile.open(QIODevice::WriteOnly);
    accessTokenFile.write(accessToken.toUtf8());
    accessTokenFile.close();
}

void PersistentTokenStorage::persistRefreshToken(QString refreshToken)
{
    refreshTokenFile.open(QIODevice::WriteOnly);
    refreshTokenFile.write(refreshToken.toUtf8());
    refreshTokenFile.close();
}
