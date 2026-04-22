#pragma once
#include <QString>
#include <QJsonObject>
#include <QMessageBox>
#include <QWidget>
#include <QMap>

class ErrorHelper {
public:
    static QString translate(const QJsonObject& response) {
        if (response["status"].toString() == "success") return {};

        int code = response["code"].toInt();
        QString serverMsg = response["message"].toString();

        static const QMap<QString, QString> translations = {
            {"Invalid email or password", "Неверный email или пароль"},
            {"Email already registered", "Этот email уже зарегистрирован"},
            {"Slot is already booked or invalid", "Слот уже забронирован или недоступен"},
            {"Failed to add slot", "Не удалось добавить слот"},
            {"Cannot delete slot: not found, not owned, or already booked", "Невозможно удалить слот: не найден, не принадлежит вам или уже забронирован"},
            {"Failed to send message", "Не удалось отправить сообщение"},
            {"Session not found", "Сессия не найдена"},
        };

        if (translations.contains(serverMsg)) {
            return translations[serverMsg];
        }

        static const QMap<int, QString> codeMessages = {
            {400, "Некорректный запрос"},
            {401, "Ошибка авторизации"},
            {404, "Не найдено"},
            {409, "Конфликт данных"},
            {422, "Ошибка валидации"},
            {500, "Внутренняя ошибка сервера"},
        };

        if (codeMessages.contains(code)) {
            return codeMessages[code];
        }

        if (!serverMsg.isEmpty()) {
            return serverMsg;
        }

        return "Произошла неизвестная ошибка";
    }

    static QString fieldErrors(const QJsonObject& response) {
        if (!response.contains("field_errors")) return {};

        QJsonObject errors = response["field_errors"].toObject();
        QStringList messages;

        static const QMap<QString, QString> fieldNames = {
            {"email", "Email"},
            {"password", "Пароль"},
            {"name", "Имя"},
            {"specialization", "Специализация"},
            {"education", "Образование"},
            {"description", "Описание"},
            {"slot_id", "ID слота"},
            {"psychologist_id", "ID психолога"},
            {"session_id", "ID сессии"},
            {"sender_id", "ID отправителя"},
            {"text", "Текст сообщения"},
            {"start_time", "Время начала"},
        };

        for (auto it = errors.begin(); it != errors.end(); ++it) {
            QString field = fieldNames.value(it.key(), it.key());
            messages << QString("• %1: %2").arg(field, it.value().toString());
        }

        return messages.join("\n");
    }

    static void showError(QWidget* parent, const QJsonObject& response, const QString& title = "Ошибка") {
        QString msg = translate(response);
        QString fields = fieldErrors(response);

        if (!fields.isEmpty()) {
            msg += "\n\n" + fields;
        }

        QMessageBox::warning(parent, title, msg);
    }

    static bool isSuccess(const QJsonObject& response) {
        return response["status"].toString() == "success";
    }

    static void showNetworkError(QWidget* parent) {
        QMessageBox::critical(parent, "Ошибка сети", "Не удалось подключиться к серверу.\nПроверьте, что сервер запущен.");
    }
};

