#include <QtTest>
#include <QJsonObject>
#include "../src/network/ErrorHelper.h"

class ClientLogicTest : public QObject {
    Q_OBJECT
private slots:
    void testErrorTranslation() {
        QJsonObject resp;
        resp["status"] = "error";
        resp["message"] = "Invalid email or password";
        
        QString translated = ErrorHelper::translate(resp);
        QCOMPARE(translated, QString("Неверный email или пароль"));
    }
    
    void testErrorByCode() {
        QJsonObject resp;
        resp["status"] = "error";
        resp["code"] = 404;
        
        QString translated = ErrorHelper::translate(resp);
        QCOMPARE(translated, QString("Не найдено"));
    }
    
    void testFieldErrorsFormatting() {
        QJsonObject resp;
        QJsonObject fields;
        fields["email"] = "Required";
        fields["password"] = "Too short";
        resp["field_errors"] = fields;
        
        QString errors = ErrorHelper::fieldErrors(resp);
        QVERIFY(errors.contains("Email: Required"));
        QVERIFY(errors.contains("Пароль: Too short"));
    }
};

QTEST_GUILESS_MAIN(ClientLogicTest)
#include "test_client_logic.moc"
