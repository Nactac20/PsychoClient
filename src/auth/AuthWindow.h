#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>

#include <QFormLayout>

class AuthWindow : public QWidget {
    Q_OBJECT
public:
    explicit AuthWindow(QWidget* parent = nullptr);
    
signals:
    void loginSuccess(const QJsonObject& data);
    
private slots:
    void onLogin();
    void onRegister();
    void switchToLogin();
    void switchToRegister();
    void onRoleChanged(int index);
    
private:
    void setupLoginPage();
    void setupRegisterPage();
    void showError(const QString& message);
    void highlightFields(const QJsonObject& fieldErrors);
    void clearHighlights();
    
    QStackedWidget* m_stackedWidget;
    
    QLineEdit* m_loginEmail;
    QLineEdit* m_loginPassword;
    QPushButton* m_loginButton;
    QPushButton* m_toRegisterButton;
    QLabel* m_loginError;
    
    QLineEdit* m_regName;
    QLineEdit* m_regEmail;
    QLineEdit* m_regPassword;
    QComboBox* m_regRole;
    QLineEdit* m_regSpecialization;
    QLineEdit* m_regEducation;
    QTextEdit* m_regDescription;
    QPushButton* m_registerButton;
    QPushButton* m_toLoginButton;
    QLabel* m_regError;
    QFormLayout* m_regForm;
};
