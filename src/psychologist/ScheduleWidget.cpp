#include "ScheduleWidget.h"
#include "../network/NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDate>

ScheduleWidget::ScheduleWidget(int psychologistId, QWidget* parent)
    : QWidget(parent)
    , m_psychologistId(psychologistId) {

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    QVBoxLayout* calendarLayout = new QVBoxLayout;
    QLabel* calendarLabel = new QLabel("Календарь расписания");
    calendarLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    calendarLayout->addWidget(calendarLabel);

    m_calendar = new QCalendarWidget;
    m_calendar->setMinimumDate(QDate::currentDate());
    calendarLayout->addWidget(m_calendar);
    mainLayout->addLayout(calendarLayout, 1);

    QVBoxLayout* slotsLayout = new QVBoxLayout;
    m_statusLabel = new QLabel("Выберите дату в календаре");
    m_statusLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    slotsLayout->addWidget(m_statusLabel);

    m_slotsTable = new QTableWidget;
    m_slotsTable->setColumnCount(4);
    m_slotsTable->setHorizontalHeaderLabels({"ID", "Время", "Длительность", "Формат"});
    m_slotsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_slotsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_slotsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    slotsLayout->addWidget(m_slotsTable);
    mainLayout->addLayout(slotsLayout, 1);

    connect(m_calendar, &QCalendarWidget::selectionChanged, [this]() {
        onDateSelected(m_calendar->selectedDate());
    });
    connect(m_slotsTable, &QTableWidget::cellClicked, [this](int row, int) {
        if (row >= 0 && m_slotsTable->item(row, 0)) {
            int slotId = m_slotsTable->item(row, 0)->text().toInt();
            emit slotSelected(slotId);
        }
    });
}

void ScheduleWidget::refresh() {
    onDateSelected(m_calendar->selectedDate());
}

void ScheduleWidget::onDateSelected(const QDate& date) {
    m_statusLabel->setText(QString("Слоты на %1").arg(date.toString("dd.MM.yyyy")));
    loadSlotsForDate(date);
}

void ScheduleWidget::loadSlotsForDate(const QDate& date) {
    m_slotsTable->setRowCount(0);

    QJsonObject request;
    request["action"] = "get_free_slots";
    QJsonObject data;
    data["psychologist_id"] = m_psychologistId;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this, date](const QJsonObject& response) {
        m_slotsTable->setRowCount(0);

        if (response["status"] != "success") return;

        QJsonArray slotList = response["data"].toArray();

        for (int i = 0; i < slotList.size(); ++i) {
            QJsonObject slot = slotList[i].toObject();
            qint64 startTime = slot["start_time"].toInteger();
            QDateTime dt = QDateTime::fromSecsSinceEpoch(startTime);

            if (dt.date() != date) continue;

            int row = m_slotsTable->rowCount();
            m_slotsTable->insertRow(row);

            m_slotsTable->setItem(row, 0, new QTableWidgetItem(QString::number(slot["slot_id"].toInt())));
            m_slotsTable->setItem(row, 1, new QTableWidgetItem(dt.toString("HH:mm")));
            m_slotsTable->setItem(row, 2, new QTableWidgetItem(QString::number(slot["duration"].toInt()) + " мин"));

            QString format = slot["format"].toString();
            m_slotsTable->setItem(row, 3, new QTableWidgetItem(format == "online" ? "Онлайн" : "Офлайн"));
        }

        if (m_slotsTable->rowCount() == 0) {
            m_statusLabel->setText(QString("Нет слотов на %1").arg(date.toString("dd.MM.yyyy")));
        }
    });
}

