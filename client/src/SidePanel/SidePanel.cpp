#include "SidePanel.h"
#include <sodium.h>

SidePanel::SidePanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    hide();
}

void SidePanel::setupUi()
{
    setFixedWidth(340);
    setObjectName("sidePanel");
    setStyleSheet(R"(
        #sidePanel {
            background-color: #181825;
            border-left: 1px solid #313244;
        }
        QLabel#panelTitle {
            color: #cdd6f4;
            font-size: 16px;
            font-weight: bold;
            font-family: 'JetBrains Mono', monospace;
        }
        QLabel#fieldLabel {
            color: #a6adc8;
            font-size: 12px;
            margin-bottom: 2px;
        }
        QLabel#statusLabel {
            font-size: 12px;
            padding: 6px;
            border-radius: 6px;
        }
        QLineEdit {
            background: #1e1e2e;
            border: 1px solid #45475a;
            border-radius: 6px;
            color: #cdd6f4;
            padding: 8px 10px;
            font-size: 13px;
            font-family: 'JetBrains Mono', monospace;
        }
        QLineEdit:focus { border-color: #89b4fa; }
        QPushButton#confirmBtn {
            background: #89b4fa;
            color: #1e1e2e;
            border: none;
            border-radius: 7px;
            padding: 10px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton#confirmBtn:hover { background: #b4d0f7; }
        QPushButton#cancelBtn {
            background: transparent;
            color: #a6adc8;
            border: 1px solid #45475a;
            border-radius: 7px;
            padding: 10px;
            font-size: 13px;
        }
        QPushButton#cancelBtn:hover { background: #313244; }
        QPushButton#generateBtn {
            background: #313244;
            color: #cba6f7;
            border: 1px solid #cba6f7;
            border-radius: 6px;
            padding: 8px 10px;
            font-size: 11px;
            white-space: nowrap;
        }
        QPushButton#generateBtn:hover { background: #3d3555; }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 24, 20, 20);
    mainLayout->setSpacing(14);

    titleLabel_ = new QLabel("Dodaj hasło", this);
    titleLabel_->setObjectName("panelTitle");
    mainLayout->addWidget(titleLabel_);

    // --- Credential fields (Add/Edit) ---
    credentialFields_ = new QWidget(this);
    auto* credLayout = new QVBoxLayout(credentialFields_);
    credLayout->setContentsMargins(0, 0, 0, 0);
    credLayout->setSpacing(10);

    auto addField = [&](QWidget* container, QVBoxLayout* layout, const QString& label, QLineEdit*& field, bool isPassword = false) {
        auto* lbl = new QLabel(label, container);
        lbl->setObjectName("fieldLabel");
        field = new QLineEdit(container);
        if (isPassword) field->setEchoMode(QLineEdit::Password);
        layout->addWidget(lbl);
        layout->addWidget(field);
    };

    addField(credentialFields_, credLayout, "Nazwa serwisu", serviceInput_);
    addField(credentialFields_, credLayout, "Login / e-mail", usernameInput_);

    // Password row with generate button
    auto* passLbl = new QLabel("Hasło", credentialFields_);
    passLbl->setObjectName("fieldLabel");
    credLayout->addWidget(passLbl);

    auto* passRow = new QHBoxLayout();
    passRow->setSpacing(8);
    passwordInput_ = new QLineEdit(credentialFields_);
    passwordInput_->setEchoMode(QLineEdit::Password);
    auto* generateBtn = new QPushButton("⚙ Generuj", credentialFields_);
    generateBtn->setObjectName("generateBtn");
    generateBtn->setCursor(Qt::PointingHandCursor);
    generateBtn->setFixedWidth(90);
    passRow->addWidget(passwordInput_);
    passRow->addWidget(generateBtn);
    credLayout->addLayout(passRow);

    connect(generateBtn, &QPushButton::clicked, this, [this]() {
        QString pwd = generatePassword();
        passwordInput_->setText(pwd);
        passwordInput_->setEchoMode(QLineEdit::Normal); // pokaż wygenerowane
        setStatus("Hasło wygenerowane — możesz je skopiować.", false);
    });

    mainLayout->addWidget(credentialFields_);

    // --- Change password fields ---
    changePassFields_ = new QWidget(this);
    auto* passLayout = new QVBoxLayout(changePassFields_);
    passLayout->setContentsMargins(0, 0, 0, 0);
    passLayout->setSpacing(10);

    addField(changePassFields_, passLayout, "Aktualne hasło", oldPasswordInput_, true);
    addField(changePassFields_, passLayout, "Nowe hasło", newPasswordInput_, true);
    addField(changePassFields_, passLayout, "Powtórz nowe hasło", confirmPasswordInput_, true);
    mainLayout->addWidget(changePassFields_);

    statusLabel_ = new QLabel("", this);
    statusLabel_->setObjectName("statusLabel");
    statusLabel_->setWordWrap(true);
    statusLabel_->hide();
    mainLayout->addWidget(statusLabel_);

    mainLayout->addStretch();

    confirmBtn_ = new QPushButton("Zapisz", this);
    confirmBtn_->setObjectName("confirmBtn");
    confirmBtn_->setCursor(Qt::PointingHandCursor);

    cancelBtn_ = new QPushButton("Anuluj", this);
    cancelBtn_->setObjectName("cancelBtn");
    cancelBtn_->setCursor(Qt::PointingHandCursor);

    mainLayout->addWidget(confirmBtn_);
    mainLayout->addWidget(cancelBtn_);

    connect(confirmBtn_, &QPushButton::clicked, this, &SidePanel::onConfirmClicked);
    connect(cancelBtn_, &QPushButton::clicked, this, &SidePanel::close);
}

void SidePanel::openForAdd()
{
    mode_ = PanelMode::Add;
    editingId_ = -1;
    titleLabel_->setText("➕  Dodaj hasło");
    credentialFields_->show();
    changePassFields_->hide();
    clearInputs();
    animateOpen();
}

void SidePanel::openForEdit(const CredentialData& data)
{
    mode_ = PanelMode::Edit;
    editingId_ = data.id;
    titleLabel_->setText("✏  Edytuj wpis");
    credentialFields_->show();
    changePassFields_->hide();
    serviceInput_->setText(data.serviceName);
    usernameInput_->setText(data.loginName);
    passwordInput_->setText(data.password);
    statusLabel_->hide();
    animateOpen();
}

void SidePanel::openForChangePassword()
{
    mode_ = PanelMode::ChangePassword;
    titleLabel_->setText("🔑  Zmień hasło konta");
    credentialFields_->hide();
    changePassFields_->show();
    clearInputs();
    animateOpen();
}

void SidePanel::close()
{
    animateClose();
}

void SidePanel::onConfirmClicked()
{
    setStatus("", false);

    if (mode_ == PanelMode::Add || mode_ == PanelMode::Edit) {
        QString service = serviceInput_->text().trimmed();
        QString username = usernameInput_->text().trimmed();
        QString password = passwordInput_->text();

        if (service.isEmpty() || username.isEmpty() || password.isEmpty()) {
            setStatus("Wypełnij wszystkie pola.");
            return;
        }
        if (password.length() < 8) {
            setStatus("Hasło musi mieć min. 8 znaków.");
            return;
        }
        if (service.length() > 96 || username.length() > 64 || password.length() > 64) {
            setStatus("Przekroczono dozwoloną długość pól.");
            return;
        }

        if (mode_ == PanelMode::Add)
            emit addRequested(service, username, password);
        else
            emit editRequested(editingId_, service, username, password);

    } else {
        QString oldPass = oldPasswordInput_->text();
        QString newPass = newPasswordInput_->text();
        QString confirmPass = confirmPasswordInput_->text();

        if (oldPass.isEmpty() || newPass.isEmpty() || confirmPass.isEmpty()) {
            setStatus("Wypełnij wszystkie pola.");
            return;
        }
        if (newPass.length() < 8) {
            setStatus("Nowe hasło musi mieć min. 8 znaków.");
            return;
        }
        if (newPass != confirmPass) {
            setStatus("Nowe hasła nie są identyczne.");
            return;
        }
        emit changePasswordRequested(oldPass, newPass);
    }
}

void SidePanel::clearInputs()
{
    if (serviceInput_) serviceInput_->clear();
    if (usernameInput_) usernameInput_->clear();
    if (passwordInput_) {
        passwordInput_->clear();
        passwordInput_->setEchoMode(QLineEdit::Password);
    }
    if (oldPasswordInput_) oldPasswordInput_->clear();
    if (newPasswordInput_) newPasswordInput_->clear();
    if (confirmPasswordInput_) confirmPasswordInput_->clear();
    statusLabel_->hide();
}

void SidePanel::setStatus(const QString& msg, bool error)
{
    if (msg.isEmpty()) { statusLabel_->hide(); return; }
    statusLabel_->setText(msg);
    statusLabel_->setStyleSheet(error
        ? "color: #f38ba8; background: #2a1a1e; border-radius: 6px; padding: 6px;"
        : "color: #a6e3a1; background: #1a2a1e; border-radius: 6px; padding: 6px;");
    statusLabel_->show();
}

QString SidePanel::generatePassword(int length)
{
    // Używamy libsodium do losowania — już jest w projekcie
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*()_+-=[]{}|;:,.<>?";
    const int charsetSize = sizeof(charset) - 1;

    QString result;
    result.reserve(length);
    for (int i = 0; i < length; i++) {
        uint32_t idx = randombytes_uniform(charsetSize);
        result += charset[idx];
    }
    return result;
}

void SidePanel::animateOpen()
{
    show();
    auto* anim = new QPropertyAnimation(this, "maximumWidth");
    anim->setDuration(220);
    anim->setStartValue(0);
    anim->setEndValue(340);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    setMaximumWidth(0);
}

void SidePanel::animateClose()
{
    auto* anim = new QPropertyAnimation(this, "maximumWidth");
    anim->setDuration(180);
    anim->setStartValue(340);
    anim->setEndValue(0);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, &QPropertyAnimation::finished, this, &QWidget::hide);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}