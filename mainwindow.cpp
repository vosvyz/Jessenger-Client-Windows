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
    httpClient.setRamTokenStorage(ramTokenStorage);
    wsClient.setRamTokenStorage(ramTokenStorage);
}

MainWindow::~MainWindow()
{
    delete ramTokenStorage; // Shutdown logic there
    delete ui;
}

void MainWindow::connectSignals()
{
    using Connection = std::tuple<QObject*, const char*, QObject*, const char*>;
    std::vector<Connection> connections = {
        { &httpClient, SIGNAL(signError(QString)), this, SLOT(signError(QString)) },
        { &httpClient, SIGNAL(signProcessed()), this, SLOT(signProcessed()) },
        { &httpClient, SIGNAL(unauthorized()), this, SLOT(unauthorized()) },
        { &httpClient, SIGNAL(shouldConfirmEmail()), this, SLOT(shouldConfirmEmail()) },
        { this, SIGNAL(sign(QString, QMap<QString, QString>)), &httpClient, SLOT(sign(QString, QMap<QString, QString>)) },
        { &confirmEmailExpiredTimer, SIGNAL(timeout()), this, SLOT(confirmEmailExpired()) }
    };

    for (const auto& conn : connections) {
        QObject::connect(std::get<0>(conn), std::get<1>(conn), std::get<2>(conn), std::get<3>(conn));
    }
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

void MainWindow::unauthorized() {
    ui->stackedWidget->setCurrentIndex(pageNameToIndexMap["signPage"]);
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
    ui->stackedWidget->setCurrentIndex(pageNameToIndexMap["confirmEmailPage"]);
}

void MainWindow::confirmEmailExpired()
{
    QString warning = "You didn't confirm your email!";
    if (authType == AuthorizationType::Login) {
        ui->stackedWidget->setCurrentIndex(pageNameToIndexMap["signInFormPage"]);
        displaySignInWarning(warning);
    }
    if (authType == AuthorizationType::Registration) {
        ui->stackedWidget->setCurrentIndex(pageNameToIndexMap["signUpFormPage"]);
        displaySignUpWarning(warning);
    }
}

void MainWindow::on_goToSignInButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(pageNameToIndexMap["signInFormPage"]);
}

void MainWindow::on_goToSignUpButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(pageNameToIndexMap["signUpFormPage"]);
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

