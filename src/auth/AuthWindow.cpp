#include "AuthWindow.h"
#include "../network/NetworkManager.h"
#include "../network/ErrorHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

AuthWindow::AuthWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("PsychoClient - Вход");
    setFixedSize(450, 650);
    
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName("authStacked");
    
    setupLoginPage();
    setupRegisterPage();
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_stackedWidget);
    
    switchToLogin();
}

void AuthWindow::setupLoginPage() {
    QWidget* page = new QWidget;
    
    QLabel* title = new QLabel("Вход в систему");
    title->setStyleSheet("font-size: 18px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    
    m_loginEmail = new QLineEdit;
    m_loginEmail->setPlaceholderText("email@example.com");
    
    m_loginPassword = new QLineEdit;
    m_loginPassword->setPlaceholderText("Пароль");
    m_loginPassword->setEchoMode(QLineEdit::Password);
    
    m_loginButton = new QPushButton("Войти");
    m_toRegisterButton = new QPushButton("Нет аккаунта? Зарегистрироваться");
    m_loginError = new QLabel;
    m_loginError->setStyleSheet("color: red;");
    m_loginError->setVisible(false);
    
    QFormLayout* form = new QFormLayout;
    form->addRow("Email:", m_loginEmail);
    form->addRow("Пароль:", m_loginPassword);
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(15);
    
    layout->addStretch();
    layout->addWidget(title);
    layout->addSpacing(30);
    layout->addLayout(form);
    layout->addSpacing(10);
    layout->addWidget(m_loginButton);
    layout->addWidget(m_loginError);
    layout->addSpacing(20);
    layout->addWidget(m_toRegisterButton);
    layout->addStretch();
    
    connect(m_loginButton, &QPushButton::clicked, this, &AuthWindow::onLogin);
    connect(m_toRegisterButton, &QPushButton::clicked, this, &AuthWindow::switchToRegister);
    
    m_stackedWidget->addWidget(page);
}

void AuthWindow::setupRegisterPage() {
    QWidget* page = new QWidget;
    
    QLabel* title = new QLabel("Регистрация");
    title->setStyleSheet("font-size: 18px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    
    m_regName = new QLineEdit;
    m_regName->setPlaceholderText("Иван Иванов");
    
    m_regEmail = new QLineEdit;
    m_regEmail->setPlaceholderText("email@example.com");
    
    m_regPassword = new QLineEdit;
    m_regPassword->setPlaceholderText("Пароль (минимум 6 символов)");
    m_regPassword->setEchoMode(QLineEdit::Password);
    
    m_regRole = new QComboBox;
    m_regRole->addItem("Клиент", "client");
    m_regRole->addItem("Психолог", "psychologist");
    
    m_regSpecialization = new QLineEdit;
    m_regSpecialization->setPlaceholderText("Специализация (обязательно для психолога)");
    
    m_regEducation = new QLineEdit;
    m_regEducation->setPlaceholderText("Образование (обязательно для психолога)");
    
    m_regDescription = new QTextEdit;
    m_regDescription->setPlaceholderText("Описание опыта работы (обязательно для психолога)");
    m_regDescription->setMaximumHeight(80);
    
    m_registerButton = new QPushButton("Зарегистрироваться");
    m_toLoginButton = new QPushButton("Уже есть аккаунт? Войти");
    m_regError = new QLabel;
    m_regError->setStyleSheet("color: red;");
    m_regError->setVisible(false);
    
    QFormLayout* form = new QFormLayout;
    form->addRow("Имя:", m_regName);
    form->addRow("Email:", m_regEmail);
    form->addRow("Пароль:", m_regPassword);
    form->addRow("Роль:", m_regRole);
    form->addRow("Специализация:", m_regSpecialization);
    form->addRow("Образование:", m_regEducation);
    form->addRow("Описание:", m_regDescription);
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(10);
    
    layout->addWidget(title);
    layout->addSpacing(10);
    layout->addLayout(form);
    layout->addSpacing(10);
    layout->addWidget(m_registerButton);
    layout->addWidget(m_regError);
    layout->addSpacing(10);
    layout->addWidget(m_toLoginButton);
    layout->addStretch();
    
    connect(m_registerButton, &QPushButton::clicked, this, &AuthWindow::onRegister);
    connect(m_toLoginButton, &QPushButton::clicked, this, &AuthWindow::switchToLogin);
    connect(m_regRole, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AuthWindow::onRoleChanged);
    onRoleChanged(0);
    
    m_stackedWidget->addWidget(page);
}

void AuthWindow::onRoleChanged(int index) {
    bool isPsychologist = (index == 1);
    m_regSpecialization->setVisible(isPsychologist);
    m_regEducation->setVisible(isPsychologist);
    m_regDescription->setVisible(isPsychologist);
}

void AuthWindow::switchToLogin() {
    m_stackedWidget->setCurrentIndex(0);
    clearHighlights();
}

void AuthWindow::switchToRegister() {
    m_stackedWidget->setCurrentIndex(1);
    clearHighlights();
}

void AuthWindow::onLogin() {
    QString email = m_loginEmail->text().trimmed();
    QString password = m_loginPassword->text();
    
    if (email.isEmpty() || password.isEmpty()) {
        showError("Заполните все поля");
        return;
    }
    
    m_loginButton->setEnabled(false);
    m_loginError->setVisible(false);
    
    QJsonObject request;
    request["action"] = "login";
    QJsonObject data;
    data["email"] = email;
    data["password"] = password;
    request["data"] = data;
    
    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        m_loginButton->setEnabled(true);
        
        if (response["status"] == "success") {
            emit loginSuccess(response["data"].toObject());
        } else {
            showError(ErrorHelper::translate(response));
        }
    });
}

void AuthWindow::onRegister() {
    QString name = m_regName->text().trimmed();
    QString email = m_regEmail->text().trimmed();
    QString password = m_regPassword->text();
    QString role = m_regRole->currentData().toString();
    
    if (name.isEmpty() || email.isEmpty() || password.isEmpty()) {
        showError("Заполните обязательные поля");
        return;
    }
    
    if (password.length() < 6) {
        showError("Пароль должен быть не менее 6 символов");
        return;
    }
    
    QJsonObject request;
    request["action"] = "register";
    QJsonObject data;
    data["name"] = name;
    data["email"] = email;
    data["password"] = password;
    data["role"] = role;
    
    if (role == "psychologist") {
        QString spec = m_regSpecialization->text().trimmed();
        QString edu = m_regEducation->text().trimmed();
        QString desc = m_regDescription->toPlainText().trimmed();
        
        if (spec.isEmpty() || edu.isEmpty() || desc.isEmpty()) {
            showError("Заполните все поля для психолога");
            return;
        }
        data["specialization"] = spec;
        data["education"] = edu;
        data["description"] = desc;
    }
    
    request["data"] = data;
    
    m_registerButton->setEnabled(false);
    m_regError->setVisible(false);
    
    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        m_registerButton->setEnabled(true);
        
        if (response["status"] == "success") {
            QMessageBox::information(this, "Успех", "Регистрация прошла успешно!\nТеперь войдите в систему.");
            switchToLogin();
            m_loginEmail->setText(m_regEmail->text());
            m_loginPassword->clear();
            clearHighlights();
        } else {
            if (response.contains("field_errors")) {
                highlightFields(response["field_errors"].toObject());
            }
            showError(ErrorHelper::translate(response));
        }
    });
}

void AuthWindow::showError(const QString& message) {
    QLabel* errorLabel = (m_stackedWidget->currentIndex() == 0) ? m_loginError : m_regError;
    errorLabel->setText(message);
    errorLabel->setVisible(true);
}

void AuthWindow::highlightFields(const QJsonObject& fieldErrors) {
    clearHighlights();
    
    if (fieldErrors.contains("email")) {
        m_regEmail->setStyleSheet("border: 1px solid red;");
    }
    if (fieldErrors.contains("name")) {
        m_regName->setStyleSheet("border: 1px solid red;");
    }
    if (fieldErrors.contains("password")) {
        m_regPassword->setStyleSheet("border: 1px solid red;");
    }
    if (fieldErrors.contains("specialization")) {
        m_regSpecialization->setStyleSheet("border: 1px solid red;");
    }
    if (fieldErrors.contains("education")) {
        m_regEducation->setStyleSheet("border: 1px solid red;");
    }
    if (fieldErrors.contains("description")) {
        m_regDescription->setStyleSheet("border: 1px solid red;");
    }
}

void AuthWindow::clearHighlights() {
    QString defaultStyle = "";
    m_regEmail->setStyleSheet(defaultStyle);
    m_regName->setStyleSheet(defaultStyle);
    m_regPassword->setStyleSheet(defaultStyle);
    m_regSpecialization->setStyleSheet(defaultStyle);
    m_regEducation->setStyleSheet(defaultStyle);
    m_regDescription->setStyleSheet(defaultStyle);
}
