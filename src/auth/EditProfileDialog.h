#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

class EditProfileDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditProfileDialog(int userId, const QString& role, QWidget* parent = nullptr);

    void setProfileData(const QString& name, const QString& email, const QString& spec = "", const QString& edu = "", const QString& desc = "");

    QString getName() const;
    QString getEmail() const;
    QString getPassword() const;
    QString getSpec() const;
    QString getEdu() const;
    QString getDesc() const;

signals:
    void accountDeleted();

private slots:
    void onSave();
    void onDeleteAccount();

private:
    int m_userId;
    QString m_role;

    QLineEdit* m_nameInput;
    QLineEdit* m_emailInput;
    QLineEdit* m_passwordInput;
    QLineEdit* m_specInput;
    QLineEdit* m_eduInput;
    QTextEdit* m_descInput;

    QLabel* m_specLabel;
    QLabel* m_eduLabel;
    QLabel* m_descLabel;
};

