#pragma once
#include <QWidget>
#include <QCalendarWidget>
#include <QTableWidget>
#include <QLabel>

class ScheduleWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScheduleWidget(int psychologistId, QWidget* parent = nullptr);
    void refresh();

signals:
    void slotSelected(int slotId);

private slots:
    void onDateSelected(const QDate& date);

private:
    void loadSlotsForDate(const QDate& date);

    int m_psychologistId;
    QCalendarWidget* m_calendar;
    QTableWidget* m_slotsTable;
    QLabel* m_statusLabel;
};

