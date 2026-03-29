#include "LoginWidget.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(50, 50, 50, 50);
    layout->setSpacing(15);

    auto* titleLabel = new QLabel("<h2>Menedżer Haseł</h2>", this);
    titleLabel->setAlignment(Qt::AlignCenter);

    loginInput = new QLineEdit(this);
    loginInput->setPlaceholderText("Nazwa użytkownika");

    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Hasło");
    passwordInput->setEchoMode(QLineEdit::Password);

    loginBtn = new QPushButton("Zaloguj", this);
    loginBtn->setCursor(Qt::PointingHandCursor);

    registerBtn = new QPushButton("Zarejestruj", this);
    registerBtn->setCursor(Qt::PointingHandCursor);

    statusLabel = new QLabel("", this);
    statusLabel->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(titleLabel);
    layout->addWidget(loginInput);
    layout->addWidget(passwordInput);
    layout->addWidget(loginBtn);
    layout->addWidget(registerBtn);
    layout->addWidget(statusLabel);
    layout->addStretch();

    networkManager = new QNetworkAccessManager(this);

    connect(loginBtn, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(registerBtn, &QPushButton::clicked, this, &LoginWidget::onRegisterClicked);
    connect(networkManager, &QNetworkAccessManager::finished, this, &LoginWidget::handleNetworkReply);
}

void LoginWidget::onLoginClicked() {
    sendAuthRequest("/login");
}

void LoginWidget::onRegisterClicked() {
    sendAuthRequest("/register");
}

void LoginWidget::sendAuthRequest(const QString& endpoint) {
    QString login = loginInput->text().trimmed();
    QString password = passwordInput->text().trimmed();

    if (login.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Podaj login i hasło!");
        statusLabel->setStyleSheet("color: #ff4444; font-weight: bold;");
        return;
    }

    statusLabel->setText("Łączenie z serwerem...");
    statusLabel->setStyleSheet("color: #aaaaaa;");

    QJsonObject json;
    json["login"] = login;
    json["password"] = password;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QUrl url("http://localhost:18080" + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    request.setRawHeader("X-Action", endpoint.toUtf8());

    networkManager->post(request, data);
}

void LoginWidget::handleNetworkReply(QNetworkReply* reply) {
    reply->deleteLater();

    QString action = reply->request().rawHeader("X-Action");
    QByteArray responseBody = reply->readAll();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = "Błąd serwera!";
        
        QJsonDocument doc = QJsonDocument::fromJson(responseBody);
        if (!doc.isNull() && doc.isObject() && doc.object().contains("error")) {
            errorMsg = doc.object()["error"].toString();
        } else {
            errorMsg = reply->errorString();
        }

        statusLabel->setText(errorMsg);
        statusLabel->setStyleSheet("color: #ff4444; font-weight: bold;");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(responseBody);

    if (action == "/register") {
        statusLabel->setText("Konto utworzone! Możesz się zalogować.");
        statusLabel->setStyleSheet("color: #00C851; font-weight: bold;");
        passwordInput->clear();
    } 
    else if (action == "/login") {
        if (!doc.isNull() && doc.isObject() && doc.object().contains("token")) {
            QString token = doc.object()["token"].toString();
            
            statusLabel->clear();
            loginInput->clear();
            passwordInput->clear();
            
            emit loginSuccessful(token);
        } else {
            statusLabel->setText("Błąd: Serwer nie zwrócił tokenu!");
            statusLabel->setStyleSheet("color: #ff4444; font-weight: bold;");
        }
    }
}