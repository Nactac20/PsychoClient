#include "SessionCalendarDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextCharFormat>
#include <QDateTime>

SessionCalendarDialog::SessionCalendarDialog(const QJsonArray& sessions, const QString& mode, QWidget* parent)
    : QDialog(parent), m_sessions(sessions), m_mode(mode) {

    setWindowTitle("Календарь сессий");
    resize(700, 450);

    for (const auto& item : m_sessions) {
        QJsonObject s = item.toObject();
        qint64 startTime = s["start_time"].toVariant().toLongLong();
        if (startTime == 0) continue;

        QDate date = QDateTime::fromSecsSinceEpoch(startTime).date();
        m_sessionsByDate[date].append(s);
    }

    setupUI();
    highlightDates();
    onDateSelected();
}

void SessionCalendarDialog::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);

    m_calendar = new QCalendarWidget;
    m_calendar->setGridVisible(true);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    auto* rightLayout = new QVBoxLayout;
    QLabel* label = new QLabel("Детали на выбранный день:");
    label->setStyleSheet("font-weight: bold;");

    m_detailsList = new QListWidget;
    m_detailsList->setStyleSheet("background-color: white; border: 1px solid #CBD5E0; border-radius: 6px;");

    rightLayout->addWidget(label);
    rightLayout->addWidget(m_detailsList);

    mainLayout->addWidget(m_calendar, 2);
    mainLayout->addLayout(rightLayout, 1);

    connect(m_calendar, &QCalendarWidget::selectionChanged, this, &SessionCalendarDialog::onDateSelected);
}

void SessionCalendarDialog::highlightDates() {
    QTextCharFormat format;
    format.setBackground(QColor("#BEE3F8"));
    format.setForeground(QColor("#2C5282"));
    format.setFontWeight(QFont::Bold);

    for (auto it = m_sessionsByDate.begin(); it != m_sessionsByDate.end(); ++it) {
        m_calendar->setDateTextFormat(it.key(), format);
    }
}

void SessionCalendarDialog::onDateSelected() {
    if (!m_detailsList || !m_calendar) return;
    m_detailsList->clear();
    QDate date = m_calendar->selectedDate();

    if (m_sessionsByDate.contains(date)) {
        for (const auto& s : m_sessionsByDate[date]) {
            qint64 startTime = s["start_time"].toVariant().toLongLong();
            QDateTime dt = QDateTime::fromSecsSinceEpoch(startTime);
            QString timeStr = dt.isValid() ? dt.toString("HH:mm") : "??:??";

            QString name;
            if (m_mode == "client") {
                name = s["psychologist_name"].toString();
            } else {
                name = s["client_name"].toString();
            }

            QString status = s["status"].toString();
            if (status == "scheduled") status = "Назначена";
            else if (status == "completed") status = "Завершена";
            else if (status == "cancelled") status = "Отменена";

            m_detailsList->addItem(QString("%1 — %2\nСтатус: %3").arg(timeStr, name, status));
        }
    } else {
        m_detailsList->addItem("Нет сессий на этот день");
    }
}

