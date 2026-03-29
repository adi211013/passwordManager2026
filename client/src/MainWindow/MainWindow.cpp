#include "MainWindow.h"
#include "../LoginWidget/LoginWidget.h"
#include "../VaultWidget/VaultWidget.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Password Manager 2026");

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    loginWidget = new LoginWidget(this);
    vaultWidget = new VaultWidget(this);

    stackedWidget->addWidget(loginWidget);
    stackedWidget->addWidget(vaultWidget);

    connect(loginWidget, &LoginWidget::loginSuccessful, this, &MainWindow::onLoginSuccessful);
    
    connect(vaultWidget, &VaultWidget::logoutRequested, this, &MainWindow::onLogoutRequested);

    stackedWidget->setCurrentIndex(0);
}

void MainWindow::onLoginSuccessful(const QString& token) {
    vaultWidget->setAuthToken(token);
    
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onLogoutRequested() {
    vaultWidget->clearData();
    
    stackedWidget->setCurrentIndex(0);
}