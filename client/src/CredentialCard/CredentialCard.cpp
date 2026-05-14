#include "CredentialCard.h"
#include <QMessageBox>

CredentialCard::CredentialCard(const CredentialData& data, QWidget* parent)
    : QWidget(parent), data_(data)
{
    setupUi();
}

void CredentialCard::setupUi()
{
    setObjectName("credentialCard");
    setStyleSheet(R"(
        #credentialCard {
            background-color: #1e1e2e;
            border: 1px solid #313244;
            border-radius: 10px;
            padding: 4px;
        }
        #credentialCard:hover {
            border: 1px solid #585b70;
        }
        QLabel#serviceLabel {
            color: #cdd6f4;
            font-size: 15px;
            font-weight: bold;
            font-family: 'JetBrains Mono', monospace;
        }
        QLabel#loginLabel {
            color: #a6adc8;
            font-size: 12px;
            font-family: 'JetBrains Mono', monospace;
        }
        QLabel#dateLabel {
            color: #585b70;
            font-size: 11px;
        }
        QLabel#passwordLabel {
            color: #89b4fa;
            font-size: 12px;
            font-family: 'JetBrains Mono', monospace;
        }
        QPushButton {
            background: transparent;
            border: 1px solid #45475a;
            border-radius: 6px;
            color: #cdd6f4;
            padding: 4px 10px;
            font-size: 12px;
        }
        QPushButton:hover { background: #313244; }
        QPushButton#deleteBtn { color: #f38ba8; border-color: #f38ba8; }
        QPushButton#deleteBtn:hover { background: #2a1a1e; }
        QPushButton#editBtn { color: #a6e3a1; border-color: #a6e3a1; }
        QPushButton#editBtn:hover { background: #1a2a1e; }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(6);

    auto* topRow = new QHBoxLayout();
    auto* serviceLabel = new QLabel(data_.serviceName, this);
    serviceLabel->setObjectName("serviceLabel");
    topRow->addWidget(serviceLabel);
    topRow->addStretch();

    auto* editBtn = new QPushButton("✏ Edytuj", this);
    editBtn->setObjectName("editBtn");
    editBtn->setCursor(Qt::PointingHandCursor);

    auto* deleteBtn = new QPushButton("✕ Usuń", this);
    deleteBtn->setObjectName("deleteBtn");
    deleteBtn->setCursor(Qt::PointingHandCursor);

    topRow->addWidget(editBtn);
    topRow->addWidget(deleteBtn);
    mainLayout->addLayout(topRow);

    auto* loginLabel = new QLabel("👤  " + data_.loginName, this);
    loginLabel->setObjectName("loginLabel");
    mainLayout->addWidget(loginLabel);

    auto* passRow = new QHBoxLayout();
    passwordLabel_ = new QLabel("••••••••", this);
    passwordLabel_->setObjectName("passwordLabel");
    passRow->addWidget(passwordLabel_);
    passRow->addStretch();

    auto* toggleBtn = new QPushButton("👁 Pokaż", this);
    toggleBtn->setCursor(Qt::PointingHandCursor);
    passRow->addWidget(toggleBtn);
    mainLayout->addLayout(passRow);

    auto* dateLabel = new QLabel("Ostatnia zmiana: " + data_.date.left(10), this);
    dateLabel->setObjectName("dateLabel");
    mainLayout->addWidget(dateLabel);

    connect(toggleBtn, &QPushButton::clicked, this, [this, toggleBtn]() {
        togglePassword();
        toggleBtn->setText(passwordVisible_ ? "🙈 Ukryj" : "👁 Pokaż");
    });
    connect(editBtn, &QPushButton::clicked, this, [this]() {
        emit editRequested(data_);
    });
    connect(deleteBtn, &QPushButton::clicked, this, [this]() {
        auto reply = QMessageBox::question(this, "Potwierdzenie",
            "Czy na pewno chcesz usunąć wpis dla serwisu \"" + data_.serviceName + "\"?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
            emit deleteRequested(data_.id);
    });
}

void CredentialCard::togglePassword()
{
    passwordVisible_ = !passwordVisible_;
    passwordLabel_->setText(passwordVisible_ ? data_.password : "••••••••");
}
