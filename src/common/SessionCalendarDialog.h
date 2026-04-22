#pragma once
#include <QDialog>
#include <QCalendarWidget>
#include <QListWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QDate>

class SessionCalendarDialog : public QDialog {
    Q_OBJECT
public:
    explicit SessionCalendarDialog(const QJsonArray& sessions, const QString& mode, QWidget* parent = nullptr);

private slots:
    void onDateSelected();

private:
    void setupUI();
    void highlightDates();

    QCalendarWidget* m_calendar;
    QListWidget* m_detailsList;
    QJsonArray m_sessions;
    QString m_mode;
    QMap<QDate, QList<QJsonObject>> m_sessionsByDate;
};

