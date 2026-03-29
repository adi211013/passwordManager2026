#include "VaultWidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QTimer>

static const QString BASE_URL = "http://localhost:18080";

VaultWidget::VaultWidget(QWidget* parent) : QWidget(parent) {
    network_ = new QNetworkAccessManager(this);
    setupUi();
}

void VaultWidget::setupUi()
{
    setStyleSheet(R"(
        VaultWidget {
            background-color: #11111b;
        }
        QScrollArea {
            background: transparent;
            border: none;
        }
        QScrollBar:vertical {
            background: #181825;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #45475a;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QPushButton#addBtn {
            background: #a6e3a1;
            color: #1e1e2e;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: bold;
            font-family: 'JetBrains Mono', monospace;
        }
        QPushButton#addBtn:hover { background: #c3f0be; }
        QPushButton#logoutBtn {
            background: transparent;
            color: #f38ba8;
            border: 1px solid #f38ba8;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 12px;
        }
        QPushButton#logoutBtn:hover { background: #2a1a1e; }
        QPushButton#changePassBtn {
            background: transparent;
            color: #fab387;
            border: 1px solid #fab387;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 12px;
        }
        QPushButton#changePassBtn:hover { background: #2a1e1a; }
        QPushButton#refreshBtn {
            background: transparent;
            color: #89b4fa;
            border: 1px solid #89b4fa;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 12px;
        }
        QPushButton#refreshBtn:hover { background: #1a1e2a; }
        QLabel#titleLabel {
            color: #cdd6f4;
            font-size: 20px;
            font-weight: bold;
            font-family: 'JetBrains Mono', monospace;
        }
        QLabel#emptyLabel {
            color: #45475a;
            font-size: 14px;
            font-family: 'JetBrains Mono', monospace;
        }
        QLabel#statusBar {
            font-size: 12px;
            padding: 8px 14px;
            border-radius: 8px;
        }
    )");

    // Outer layout: content area + side panel
    auto* outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Main content
    auto* contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(28, 24, 28, 20);
    contentLayout->setSpacing(16);

    // Top bar
    auto* topBar = new QHBoxLayout();
    auto* titleLabel = new QLabel("🔐  Mój Sejf", this);
    titleLabel->setObjectName("titleLabel");
    topBar->addWidget(titleLabel);
    topBar->addStretch();

    auto* refreshBtn = new QPushButton("↻ Odśwież", this);
    refreshBtn->setObjectName("refreshBtn");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    auto* changePassBtn = new QPushButton("🔑 Zmień hasło", this);
    changePassBtn->setObjectName("changePassBtn");
    changePassBtn->setCursor(Qt::PointingHandCursor);

    auto* addBtn = new QPushButton("+ Dodaj hasło", this);
    addBtn->setObjectName("addBtn");
    addBtn->setCursor(Qt::PointingHandCursor);

    auto* logoutBtn = new QPushButton("Wyloguj", this);
    logoutBtn->setObjectName("logoutBtn");
    logoutBtn->setCursor(Qt::PointingHandCursor);

    topBar->addWidget(refreshBtn);
    topBar->addWidget(changePassBtn);
    topBar->addWidget(addBtn);
    topBar->addWidget(logoutBtn);
    contentLayout->addLayout(topBar);

    // Status bar
    statusBar_ = new QLabel("", this);
    statusBar_->setObjectName("statusBar");
    statusBar_->hide();
    contentLayout->addWidget(statusBar_);

    // Scroll area for cards
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    cardsContainer_ = new QWidget();
    cardsContainer_->setStyleSheet("background: transparent;");
    cardsLayout_ = new QVBoxLayout(cardsContainer_);
    cardsLayout_->setSpacing(10);
    cardsLayout_->setContentsMargins(0, 0, 0, 0);

    emptyLabel_ = new QLabel("Brak zapisanych haseł.\nKliknij '+ Dodaj hasło' żeby zacząć.", cardsContainer_);
    emptyLabel_->setObjectName("emptyLabel");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    cardsLayout_->addWidget(emptyLabel_);
    cardsLayout_->addStretch();

    scrollArea->setWidget(cardsContainer_);
    contentLayout->addWidget(scrollArea);

    outerLayout->addWidget(contentWidget, 1);

    // Side panel
    sidePanel_ = new SidePanel(this);
    outerLayout->addWidget(sidePanel_);

    // Connections
    connect(addBtn, &QPushButton::clicked, this, [this]() {
        sidePanel_->openForAdd();
    });
    connect(changePassBtn, &QPushButton::clicked, this, [this]() {
        sidePanel_->openForChangePassword();
    });
    connect(refreshBtn, &QPushButton::clicked, this, &VaultWidget::fetchCredentials);
    connect(logoutBtn, &QPushButton::clicked, this, &VaultWidget::logoutRequested);

    connect(sidePanel_, &SidePanel::addRequested, this, &VaultWidget::onAddRequested);
    connect(sidePanel_, &SidePanel::editRequested, this, &VaultWidget::onEditRequested);
    connect(sidePanel_, &SidePanel::changePasswordRequested, this, &VaultWidget::onChangePasswordRequested);
}

void VaultWidget::setAuthToken(const QString& token)
{
    authToken_ = token;
    fetchCredentials();
}

void VaultWidget::clearData()
{
    authToken_.clear();
    clearCards();
    sidePanel_->close();
    statusBar_->hide();
}

QNetworkRequest VaultWidget::makeRequest(const QString& path) const
{
    QNetworkRequest req(QUrl(BASE_URL + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + authToken_).toUtf8());
    return req;
}

void VaultWidget::clearCards()
{
    // Remove all CredentialCard widgets
    QList<CredentialCard*> cards = cardsContainer_->findChildren<CredentialCard*>();
    for (auto* card : cards) {
        cardsLayout_->removeWidget(card);
        card->deleteLater();
    }
}

void VaultWidget::showStatusBar(const QString& msg, bool error)
{
    statusBar_->setText(msg);
    statusBar_->setStyleSheet(error
        ? "color: #f38ba8; background: #2a1a1e; border-radius: 8px; padding: 8px 14px;"
        : "color: #a6e3a1; background: #1a2a1e; border-radius: 8px; padding: 8px 14px;");
    statusBar_->show();
    QTimer::singleShot(3000, statusBar_, &QLabel::hide);
}

void VaultWidget::fetchCredentials()
{
    auto* reply = network_->get(makeRequest("/vault"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            showStatusBar("Błąd pobierania haseł: " + reply->errorString(), true);
            return;
        }

        clearCards();

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray creds = doc.object()["credentials"].toArray();

        emptyLabel_->setVisible(creds.isEmpty());

        for (const auto& val : creds) {
            QJsonObject obj = val.toObject();
            CredentialData data;
            data.id = obj["id"].toInt();
            data.serviceName = obj["serviceName"].toString();
            data.loginName = obj["loginName"].toString();
            data.password = obj["password"].toString();
            data.date = obj["date"].toString();

            auto* card = new CredentialCard(data, cardsContainer_);
            // Insert before the stretch at the end
            cardsLayout_->insertWidget(cardsLayout_->count() - 1, card);

            connect(card, &CredentialCard::editRequested, this, [this](const CredentialData& d) {
                sidePanel_->openForEdit(d);
            });
            connect(card, &CredentialCard::deleteRequested, this, &VaultWidget::onDeleteRequested);
        }
    });
}

void VaultWidget::onAddRequested(const QString& service, const QString& username, const QString& password)
{
    QJsonObject body;
    body["service"] = service;
    body["username"] = username;
    body["password"] = password;

    auto* reply = network_->post(makeRequest("/vault"), QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            showStatusBar("Błąd dodawania hasła.", true);
            return;
        }
        sidePanel_->close();
        showStatusBar("Hasło zostało dodane.");
        fetchCredentials();
    });
}

void VaultWidget::onEditRequested(int id, const QString& service, const QString& username, const QString& password)
{
    QJsonObject body;
    body["service"] = service;
    body["username"] = username;
    body["password"] = password;

    auto* reply = network_->sendCustomRequest(
        makeRequest("/vault/" + QString::number(id)),
        "PUT",
        QJsonDocument(body).toJson()
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            showStatusBar("Błąd edycji wpisu.", true);
            return;
        }
        sidePanel_->close();
        showStatusBar("Wpis zaktualizowany.");
        fetchCredentials();
    });
}

void VaultWidget::onDeleteRequested(int id)
{
    auto* reply = network_->sendCustomRequest(
        makeRequest("/vault/" + QString::number(id)),
        "DELETE",
        QByteArray()
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            showStatusBar("Błąd usuwania wpisu.", true);
            return;
        }
        showStatusBar("Hasło usunięte.");
        fetchCredentials();
    });
}

void VaultWidget::onChangePasswordRequested(const QString& oldPassword, const QString& newPassword)
{
    QJsonObject body;
    body["old_password"] = oldPassword;
    body["new_password"] = newPassword;

    auto* reply = network_->sendCustomRequest(
        makeRequest("/user/password"),
        "PUT",
        QJsonDocument(body).toJson()
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            // Serwer zwraca 401 gdy stare hasło jest złe — Qt traktuje to jako error
            showStatusBar("Błąd: stare hasło jest nieprawidłowe.", true);
            return;
        }
        sidePanel_->close();
        showStatusBar("Hasło konta zostało zmienione.");
    });
}
