#include "EditProfileDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QJsonObject>
#include "../network/NetworkManager.h"
#include "../network/ErrorHelper.h"

EditProfileDialog::EditProfileDialog(int userId, const QString& role, QWidget* parent)
    : QDialog(parent), m_userId(userId), m_role(role) {

    setWindowTitle("Редактирование профиля");
    setMinimumWidth(400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout;

    m_nameInput = new QLineEdit;
    form->addRow("Имя:", m_nameInput);

    m_emailInput = new QLineEdit;
    form->addRow("Email:", m_emailInput);

    m_passwordInput = new QLineEdit;
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setPlaceholderText("Оставьте пустым, чтобы не менять");
    form->addRow("Новый пароль:", m_passwordInput);

    if (m_role == "psychologist") {
        m_specInput = new QLineEdit;
        m_eduInput = new QLineEdit;
        m_descInput = new QTextEdit;

        form->addRow("Специализация:", m_specInput);
        form->addRow("Образование:", m_eduInput);
        form->addRow("Описание:", m_descInput);
    } else {
        m_specInput = nullptr;
        m_eduInput = nullptr;
        m_descInput = nullptr;
    }

    mainLayout->addLayout(form);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* deleteBtn = new QPushButton("Удалить аккаунт");
    deleteBtn->setStyleSheet("QPushButton { background-color: #E53E3E; color: white; } QPushButton:hover { background-color: #C53030; }");

    QPushButton* cancelBtn = new QPushButton("Отмена");
    QPushButton* saveBtn = new QPushButton("Сохранить");

    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);

    mainLayout->addLayout(btnLayout);

    connect(deleteBtn, &QPushButton::clicked, this, &EditProfileDialog::onDeleteAccount);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &EditProfileDialog::onSave);
}

void EditProfileDialog::setProfileData(const QString& name, const QString& email, const QString& spec, const QString& edu, const QString& desc) {
    m_nameInput->setText(name);
    m_emailInput->setText(email);
    if (m_role == "psychologist") {
        m_specInput->setText(spec);
        m_eduInput->setText(edu);
        m_descInput->setText(desc);
    }
}

QString EditProfileDialog::getName() const { return m_nameInput->text().trimmed(); }
QString EditProfileDialog::getEmail() const { return m_emailInput->text().trimmed(); }
QString EditProfileDialog::getPassword() const { return m_passwordInput->text(); }
QString EditProfileDialog::getSpec() const { return m_specInput ? m_specInput->text().trimmed() : ""; }
QString EditProfileDialog::getEdu() const { return m_eduInput ? m_eduInput->text().trimmed() : ""; }
QString EditProfileDialog::getDesc() const { return m_descInput ? m_descInput->toPlainText().trimmed() : ""; }

void EditProfileDialog::onSave() {
    if (getName().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя не может быть пустым");
        return;
    }
    if (getEmail().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Email не может быть пустым");
        return;
    }
    accept();
}

void EditProfileDialog::onDeleteAccount() {
    auto result = QMessageBox::critical(this, "Удаление аккаунта",
                                      "Вы уверены, что хотите ПОЛНОСТЬЮ удалить свой аккаунт?\n"
                                      "Все ваши данные, записи и сообщения будут удалены навсегда.",
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        QJsonObject request;
        request["action"] = "delete_user";
        QJsonObject data;
        data["user_id"] = m_userId;
        request["data"] = data;

        NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
            if (response["status"] == "success") {
                QMessageBox::information(this, "Успех", "Ваш аккаунт был успешно удален. Приложение будет перезапущено.");
                emit accountDeleted();
                accept();
            } else {
                QMessageBox::warning(this, "Ошибка", ErrorHelper::translate(response));
            }
        });
    }
}
