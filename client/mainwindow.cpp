#include <mainwindow.h>
#include <./ui_mainwindow.h>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connectSignals();

    fillPageNameToIndexMap();

    confirmEmailExpiredTimer.setSingleShot(true);
    confirmEmailExpiredTimer.setInterval(300000);

    QMovie *loadingGif = new QMovie("assets/signing.gif");
    ui->signInWaitingLabel->setMovie(loadingGif);
    ui->signInWaitingLabel->hide();
    ui->signUpWaitingLabel->setMovie(loadingGif);
    ui->signUpWaitingLabel->hide();

    basicFont.setFamily("Segoe UI");
    basicFont.setPointSize(16);

    ramTokenStorage = new RamTokenStorage();

    networkThread.start();

    httpClient.setRamTokenStorage(ramTokenStorage);
    httpClient.moveToThread(&networkThread);

    wsClient.setRamTokenStorage(ramTokenStorage);
    wsClient.moveToThread(&networkThread);

    // load chats from cache
}

MainWindow::~MainWindow()
{
    delete ramTokenStorage;
    delete ui;
    wsClient.deleteLater();
    httpClient.deleteLater();
    networkThread.quit();
    networkThread.wait();
}

void MainWindow::connectSignals()
{
    QObject::connect(&networkThread, &QThread::started, &httpClient, &HttpClient::checkRefreshToken);

    QObject::connect(&httpClient, &HttpClient::refreshTokenVerified, &wsClient, &WsClient::initialize);

    QObject::connect(&wsClient, &WsClient::unauthorized, this, &MainWindow::unauthorized);
    QObject::connect(&httpClient, &HttpClient::unauthorized, this, &MainWindow::unauthorized);

    QObject::connect(&wsClient, &WsClient::socketConnected, this, &MainWindow::wsConnected);
    QObject::connect(&wsClient, &WsClient::socketDisconnected, this, &MainWindow::wsDisconnected);

    QObject::connect(&httpClient, &HttpClient::signError, this, &MainWindow::signError);
    QObject::connect(&httpClient, &HttpClient::signProcessed, this, &MainWindow::signProcessed);
    QObject::connect(&httpClient, &HttpClient::shouldConfirmEmail, this, &MainWindow::shouldConfirmEmail);
    QObject::connect(&httpClient, &HttpClient::getUserChatsProcessed, this, &MainWindow::getUserChatsProcessed);

    QObject::connect(&confirmEmailExpiredTimer, &QTimer::timeout, this, &MainWindow::confirmEmailExpired);

    QObject::connect(this, &MainWindow::getUserChats, &httpClient, &HttpClient::getUserChatsProxy);
    QObject::connect(this, &MainWindow::findChats, &httpClient, &HttpClient::findChatsProxy);
}

void MainWindow::fillPageNameToIndexMap()
{
    pageNameToIndexMap["signPage"] = 0;
    pageNameToIndexMap["signInFormPage"] = 1;
    pageNameToIndexMap["signUpFormPage"] = 2;
    pageNameToIndexMap["homePage"] = 3;
    pageNameToIndexMap["confirmEmailPage"] = 4;
    pageNameToIndexMap["createGroupPage"] = 5;
}

void MainWindow::wsConnected() {
    ui->connectionStatement->setText("Connected!");
}

void MainWindow::wsDisconnected() {
    ui->connectionStatement->setText("Connecting...");
}

void MainWindow::clearChats() {
    QLayoutItem* item;
    while ((item = ui->chats->takeAt(0))) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void MainWindow::getUserChatsProcessed(QJsonArray data) {
    // clear chats
    for (int i = 0; i < data.size(); ++i) {
        QJsonObject object = data[i].toObject();
        qlonglong chatId = object["chatId"].toVariant().toLongLong();
        bool group = object["group"].toBool();
        QString name = object["chatName"].toString();
        QString lastMessage = object["lastMessageText"].toString();
        qlonglong timeAsLong = object["lastMessageTime"].toVariant().toLongLong();
        QDateTime timeAsQDT = (QDateTime::fromMSecsSinceEpoch(timeAsLong, Qt::UTC)).toLocalTime();
        QString time = timeAsQDT.toString("dd.MM.yy, hh:mm");
        int neededHeight = 60; // The button height, px. Will be changed to 130 if chat has any messages to display as the last message

        ChatPushButton *button = new ChatPushButton(chatId, name, group, this);

        QLabel *nameLabel = new QLabel(button);
        nameLabel->setStyleSheet("border: none;");
        nameLabel->setText(name);
        nameLabel->setGeometry(10, 10, 580, 40);

        QLabel *lastMessageLabel = new QLabel(button);
        lastMessageLabel->setObjectName("lastMessageLabel");
        lastMessageLabel->setStyleSheet("font: 14pt \"Segoe UI\";"
                                        "color: rgb(180, 180, 180);"
                                        "border: none;");
        lastMessageLabel->setGeometry(10, 60, 580, 60);
        if (!lastMessage.isEmpty()) {
            neededHeight = 130;
            QString lastMessageSender = object["lastMessageSenderName"].toString();
            QString lastMessageInfo = lastMessageSender + ": " + lastMessage.left(30);
            if (lastMessage.size() > 30) {
                lastMessageInfo += "...";
            }
            lastMessageInfo += "\n" + time;
            lastMessageLabel->setText(lastMessageInfo);
        }

        button->setStyleSheet("font: 16pt \"Segoe UI\";"
                                  "border-bottom: 1px solid rgb(180, 180, 180);");
        button->setFixedSize(580, neededHeight);
        yourChats.append(button);
        if (ui->findUserLineEdit->text().isEmpty()) {
            ui->chats->addWidget(button);
        }
    }
}

int MainWindow::calculateLineCount(const QString& text, const QFontMetrics& metrics, int labelWidth) {
   QStringList words = text.split(' ');
   int lineCount = 1;
   QString currentLine;
   for (const QString& word : words) {
       if (metrics.horizontalAdvance(currentLine + (currentLine.isEmpty() ? "" : " ") + word) > labelWidth) {
           ++lineCount;
           currentLine = word;
       } else {
           currentLine += (currentLine.isEmpty() ? "" : " ") + word;
       }
   }
   return lineCount;
}

void MainWindow::signProcessed() {
    int homePageIndex = pageNameToIndexMap["homePage"];
    ui->stackedWidget->setCurrentIndex(homePageIndex);
}

void MainWindow::unauthorized() {
    int currentIndex = ui->stackedWidget->currentIndex();
    int signPageIndex = pageNameToIndexMap["signPage"];
    int signInFormPageIndex = pageNameToIndexMap["signInFormPage"];
    int signUpFormPageIndex = pageNameToIndexMap["signUpFormPage"];
    if (currentIndex != signPageIndex && currentIndex != signInFormPageIndex && currentIndex != signUpFormPageIndex) {
        ui->stackedWidget->setCurrentIndex(signPageIndex);
    }
}

void MainWindow::signError(QString error) {
    if (authType == AuthorizationType::Login) {
        ui->signInWaitingLabel->movie()->stop();
        ui->signInWaitingLabel->hide();
        ui->signInButton->setEnabled(true);
        displaySignInWarning(error);
    }
    if (authType == AuthorizationType::Registration) {
        ui->signUpWaitingLabel->movie()->stop();
        ui->signUpWaitingLabel->hide();
        ui->signUpButton->setEnabled(true);
        displaySignUpWarning(error);
    }
}

void MainWindow::shouldConfirmEmail()
{
    if (authType == AuthorizationType::Login) {
        ui->signInWaitingLabel->movie()->stop();
        ui->signInWaitingLabel->hide();
        ui->signInFormWrap->setGeometry(810, 430, 300, 200);
        ui->signInWaitingLabel->move(860, 230);
        ui->signInButton->setEnabled(true);
    }
    if (authType == AuthorizationType::Registration) {
        ui->signUpWaitingLabel->movie()->stop();
        ui->signUpWaitingLabel->hide();
        ui->signUpFormWrap->setGeometry(810, 405, 300, 250);
        ui->signUpWaitingLabel->move(860, 200);
        ui->signUpButton->setEnabled(true);
    }
    confirmEmailExpiredTimer.start();
    int confirmEmailPageIndex = pageNameToIndexMap["confirmEmailPage"];
    ui->stackedWidget->setCurrentIndex(confirmEmailPageIndex);
}

void MainWindow::confirmEmailExpired()
{
    QString warning = "You didn't confirm your email!";
    if (authType == AuthorizationType::Login) {
        int signInFormPageIndex = pageNameToIndexMap["signInFormPage"];
        ui->stackedWidget->setCurrentIndex(signInFormPageIndex);
        displaySignInWarning(warning);
    }
    if (authType == AuthorizationType::Registration) {
        int signUpFormPageIndex = pageNameToIndexMap["signUpFormPage"];
        ui->stackedWidget->setCurrentIndex(signUpFormPageIndex);
        displaySignUpWarning(warning);
    }
}

void MainWindow::on_goToSignInButton_clicked()
{
    int signInFormPageIndex = pageNameToIndexMap["signInFormPage"];
    ui->stackedWidget->setCurrentIndex(signInFormPageIndex);
}

void MainWindow::on_goToSignUpButton_clicked()
{
    int signUpFormPageIndex = pageNameToIndexMap["signUpFormPage"];
    ui->stackedWidget->setCurrentIndex(signUpFormPageIndex);
}

void MainWindow::displaySignInWarning(QString warning)
{
    QFontMetrics metrics(basicFont);
    int linesForWarning = calculateLineCount(warning, metrics, 300);
    ui->signInFormWrap->setGeometry(810, 430 - (20 * linesForWarning), 300, 200 + (40 * linesForWarning));
    ui->signInWaitingLabel->move(860, 230 - (20 * linesForWarning));
    ui->signInWarning->setText(warning);
}

void MainWindow::displaySignUpWarning(QString warning)
{
    QFontMetrics metrics(basicFont);
    int linesForWarning = calculateLineCount(warning, metrics, 300);
    ui->signUpFormWrap->setGeometry(810, 405 - (20 * linesForWarning), 300, 250 + (40 * linesForWarning));
    ui->signUpWaitingLabel->move(860, 200 - (20 * linesForWarning));
    ui->signUpWarning->setText(warning);
}

void MainWindow::on_signInButton_clicked()
{
    QString email = ui->signInEmailLineEdit->text();
    QString password = ui->signInPasswordLineEdit->text();

    if (email.isEmpty() || password.isEmpty()) {
        QString warning = "Fields are empty!";
        displaySignInWarning(warning);
        return;
    }

    authType = AuthorizationType::Login;

    ui->signInWaitingLabel->show();
    ui->signInWaitingLabel->movie()->start();
    ui->signInButton->setEnabled(false);

    QMap<QString, QString> requestBody;
    requestBody["email"] = email;
    requestBody["password"] = password;
    emit sign("sign/in", requestBody);
}

void MainWindow::on_signUpButton_clicked()
{
    QString warning;
    QString name = ui->signUpUsernameLineEdit->text();
    QString email = ui->signUpEmailLineEdit->text();
    QString password = ui->signUpPasswordLineEdit->text();

    if (name.isEmpty() || email.isEmpty() || password.isEmpty()) {
        warning = "Fields are empty!";
    }
    if (name.length() > 30) {
        warning = "Too long name!";
    }
    if (name == "You") {
        warning = "Sorry, this name is reserved!";
    }
    if (!warning.isEmpty()) {
        displaySignUpWarning(warning);
        return;
    }

    authType = AuthorizationType::Registration;

    ui->signUpWaitingLabel->show();
    ui->signUpWaitingLabel->movie()->start();
    ui->signUpButton->setEnabled(false);

    QMap<QString, QString> requestBody;
    requestBody["name"] = name;
    requestBody["email"] = email;
    requestBody["password"] = password;
    emit sign("sign/up", requestBody);
}

void MainWindow::on_backToSignButton_clicked()
{
    if (authType == AuthorizationType::Login) {
        on_goToSignInButton_clicked(); // Redirect to log in page
    }
    if (authType == AuthorizationType::Login) {
        on_goToSignUpButton_clicked(); // Redirect to sign up page
    }
}
