#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

struct CredentialData {
    int id;
    QString serviceName;
    QString loginName;
    QString password;
    QString date;
};

class CredentialCard : public QWidget {
    Q_OBJECT
public:
    explicit CredentialCard(const CredentialData& data, QWidget* parent = nullptr);
    int credentialId() const { return data_.id; }

signals:
    void editRequested(const CredentialData& data);
    void deleteRequested(int id);

private slots:
    void togglePassword();

private:
    CredentialData data_;
    bool passwordVisible_ = false;
    QLabel* passwordLabel_;

    void setupUi();
};
