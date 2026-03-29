#pragma once
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "../CredentialCard/CredentialCard.h"
#include "../SidePanel/SidePanel.h"

class VaultWidget : public QWidget {
    Q_OBJECT
public:
    explicit VaultWidget(QWidget* parent = nullptr);
    void setAuthToken(const QString& token);
    void clearData();

signals:
    void logoutRequested();

private slots:
    void fetchCredentials();
    void onAddRequested(const QString& service, const QString& username, const QString& password);
    void onEditRequested(int id, const QString& service, const QString& username, const QString& password);
    void onDeleteRequested(int id);
    void onChangePasswordRequested(const QString& oldPassword, const QString& newPassword);

private:
    QString authToken_;
    QNetworkAccessManager* network_;

    QWidget* cardsContainer_;
    QVBoxLayout* cardsLayout_;
    SidePanel* sidePanel_;
    QLabel* emptyLabel_;
    QLabel* statusBar_;

    void setupUi();
    void clearCards();
    QNetworkRequest makeRequest(const QString& path) const;
    void showStatusBar(const QString& msg, bool error = false);
};
