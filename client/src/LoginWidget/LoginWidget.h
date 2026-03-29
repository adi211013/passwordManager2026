#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget* parent = nullptr);
signals:
    void loginSuccessful(const QString& token);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void handleNetworkReply(QNetworkReply* reply);
private:
    void sendAuthRequest(const QString& endpoint);
    QLineEdit* loginInput;
    QLineEdit* passwordInput;
    QPushButton* loginBtn;
    QPushButton* registerBtn;
    QLabel* statusLabel ;
    QNetworkAccessManager* networkManager;
};
