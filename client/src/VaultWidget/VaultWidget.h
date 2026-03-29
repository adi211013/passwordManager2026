#pragma once
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class VaultWidget : public QWidget {
    Q_OBJECT
public:
    explicit VaultWidget(QWidget *parent = nullptr) : QWidget(parent) {
        auto* layout = new QVBoxLayout(this);
        tokenLabel = new QLabel("Token: Brak", this);
        auto* btnLogout = new QPushButton("Wyloguj", this);

        layout->addWidget(new QLabel("Witaj w swoim Sejfie!", this));
        layout->addWidget(tokenLabel);
        layout->addWidget(btnLogout);
        layout->setAlignment(Qt::AlignCenter);

        connect(btnLogout, &QPushButton::clicked, this, &VaultWidget::logoutRequested);
    }

    void setAuthToken(const QString& token) {
        tokenLabel->setText("Twój token: " + token);
    }

    void clearData() {
        tokenLabel->setText("Token: Brak");
    }

    signals:
        void logoutRequested();

private:
    QLabel* tokenLabel;
};