#pragma once

#include <QTimer>
#include <QDebug>
#include <QMap>
#include <QMovie>
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
    void sign(QString endpoint, QMap<QString, QString> requestBody);

private slots:
    void on_goToSignInButton_clicked();
    void on_goToSignUpButton_clicked();
    void on_signInButton_clicked();
    void on_signUpButton_clicked();
    void on_backToSignButton_clicked();
    void shouldConfirmEmail();
    void confirmEmailExpired();
    void unauthorized();
    void signError(QString error);

private:
    QTimer confirmEmailExpiredTimer;
    AuthorizationType authType;
    QFont basicFont; // The most usual font for this application -- Segoe UI, 16pt.
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
