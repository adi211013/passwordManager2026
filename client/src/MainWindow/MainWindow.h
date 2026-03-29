#pragma once

#include <QMainWindow>
#include <QStackedWidget>

class LoginWidget;
class VaultWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private
    slots :

    void onLoginSuccessful(const QString& token);
    void onLogoutRequested();

private:
    QStackedWidget* stackedWidget;
    LoginWidget* loginWidget;
    VaultWidget* vaultWidget;
};
