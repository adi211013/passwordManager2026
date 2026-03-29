#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include "../CredentialCard/CredentialCard.h"

enum class PanelMode {
    Add,
    Edit,
    ChangePassword
};

class SidePanel : public QWidget {
    Q_OBJECT
public:
    explicit SidePanel(QWidget* parent = nullptr);
    void openForAdd();
    void openForEdit(const CredentialData& data);
    void openForChangePassword();
    void close();

    signals:
        void addRequested(const QString& service, const QString& username, const QString& password);
    void editRequested(int id, const QString& service, const QString& username, const QString& password);
    void changePasswordRequested(const QString& oldPassword, const QString& newPassword);

private slots:
    void onConfirmClicked();

private:
    PanelMode mode_;
    int editingId_ = -1;

    QLabel* titleLabel_;
    QLineEdit* serviceInput_;
    QLineEdit* usernameInput_;
    QLineEdit* passwordInput_;
    QLineEdit* oldPasswordInput_;
    QLineEdit* newPasswordInput_;
    QLineEdit* confirmPasswordInput_;
    QLabel* statusLabel_;
    QPushButton* confirmBtn_;
    QPushButton* cancelBtn_;

    QWidget* credentialFields_;
    QWidget* changePassFields_;

    void setupUi();
    void clearInputs();
    void setStatus(const QString& msg, bool error = true);
    void animateOpen();
    void animateClose();
    static QString generatePassword(int length = 20);
};