#pragma once

#include <QTimer>
#include <QDebug>
#include <QMap>
#include <chatpushbutton.h>
#include <QThread>
#include <QMovie>
#include <QJsonObject>
#include <QMainWindow>
#include <ramtokenstorage.h>
#include <httpclient.h>
#include <wsclient.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class AuthorizationType {
        Login,
        Registration
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void connectWsClient();
    void sign(QString endpoint, QMap<QString, QString> requestBody);
    void getUserChats();
    void findChats(QMap<QString, QString> body);

private slots:
    void wsConnected();
    void wsDisconnected();
    void on_goToSignInButton_clicked();
    void on_goToSignUpButton_clicked();
    void on_signInButton_clicked();
    void on_signUpButton_clicked();
    void on_backToSignButton_clicked();
    void unauthorized();
    void signProcessed();
    void signError(QString error);
    void shouldConfirmEmail();
    void confirmEmailExpired();
    void getUserChatsProcessed(QJsonArray data);

private:
    QVector<ChatPushButton*> yourChats;
    QThread networkThread; // This thread contains both HttpClient and WsClient. The shared thread is needed to synchronize some job of these classes
    QTimer confirmEmailExpiredTimer;
    AuthorizationType authType;
    QFont basicFont; // The most usual font for this application -- Segoe UI, 16pt.
    void clearChats();
    void connectSignals();
    Ui::MainWindow *ui;
    HttpClient httpClient;
    WsClient wsClient;
    RamTokenStorage *ramTokenStorage;
    void displaySignInWarning(QString warning);
    void displaySignUpWarning(QString warning);
    void fillPageNameToIndexMap();
    QMap<QString, int> pageNameToIndexMap;
    int calculateLineCount(const QString& text, const QFontMetrics& metrics, int labelWidth);
};
