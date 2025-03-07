#include <persistenttokenstorage.h>

PersistentTokenStorage::PersistentTokenStorage()
{
    QDir homeDirectory = QDir::home();
    if (!homeDirectory.exists("Jessenger/auth")) {
        homeDirectory.mkdir("Jessenger/auth");
    }
    refreshTokenFile.setFileName(homeDirectory.path() + "/Jessenger/auth/refreshtoken.txt");
}

QString PersistentTokenStorage::readRefreshToken()
{
    refreshTokenFile.open(QIODevice::ReadOnly);
    QString result = refreshTokenFile.readAll();
    refreshTokenFile.close();
    return result;
}

void PersistentTokenStorage::persistRefreshToken(QString refreshToken)
{
    refreshTokenFile.open(QIODevice::WriteOnly);
    refreshTokenFile.write(refreshToken.toUtf8());
    refreshTokenFile.close();
}
