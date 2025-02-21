#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QFile>
#include <QDir>
#include <QDebug>

using namespace std;

class FileManager {

public:
    FileManager();
    QString getAccessToken();
    QString getRefreshToken();
    void persistTokens(QString accessToken, QString refreshToken);

private:
    QFile *accessTokenFile;
    QFile *refreshTokenFile;
};

#endif // FILEMANAGER_H
