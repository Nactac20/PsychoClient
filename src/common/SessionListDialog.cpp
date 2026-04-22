#include "SessionListDialog.h"
#include "../network/NetworkManager.h"
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QMessageBox>

SessionListDialog::SessionListDialog(int userId, const QString& role, QWidget* parent)
    : QDialog(parent), m_userId(userId), m_role(role) {

    setWindowTitle("Мои сессии");
    resize(800, 500);

    auto* layout = new QVBoxLayout(this);

    auto* header = new QHBoxLayout;
    header->addWidget(new QLabel("Фильтр статуса:"));

    m_filter = new QComboBox;
    m_filter->addItem("Все статусы", "all");
    m_filter->addItem("Назначена", "scheduled");
    m_filter->addItem("Завершена", "completed");
    m_filter->addItem("Отменена", "cancelled");

    header->addWidget(m_filter);
    header->addStretch();

    layout->addLayout(header);

    m_table = new QTableWidget(0, 5);
    if (m_role == "client") {
        m_table->setHorizontalHeaderLabels({"ID", "Психолог", "Дата и время", "Формат", "Статус"});
    } else {
        m_table->setHorizontalHeaderLabels({"ID", "Клиент", "Дата и время", "Формат", "Статус"});
    }
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(m_table);

    m_cancelBtn = new QPushButton("Отменить сессию");
    m_cancelBtn->setEnabled(false);
    layout->addWidget(m_cancelBtn);

    auto* closeBtn = new QPushButton("Закрыть");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn);

    connect(m_filter, &QComboBox::currentTextChanged, this, &SessionListDialog::loadSessions);
    connect(m_cancelBtn, &QPushButton::clicked, this, &SessionListDialog::onCancelSession);
    connect(m_table, &QTableWidget::itemSelectionChanged, [this]() {
        int row = m_table->currentRow();
        bool canCancel = false;
        if (row >= 0) {
            QString status = m_table->item(row, 4)->text();
            canCancel = (status != "Отменена" && status != "Завершена");
        }
        m_cancelBtn->setEnabled(canCancel);
    });

    loadSessions();
}

void SessionListDialog::loadSessions() {
    QJsonObject request;
    request["action"] = "get_my_sessions";
    QJsonObject data;
    if (m_role == "client") {
        data["client_id"] = m_userId;
    } else {
        data["psychologist_id"] = m_userId;
    }
    request["data"] = data;

    QString statusFilter = m_filter->currentData().toString();

    NetworkManager::instance().sendRequest(request, [this, statusFilter](const QJsonObject& response) {
        if (response["status"] == "success") {
            QJsonArray sessions = response["data"].toArray();
            m_table->setRowCount(0);

            for (const auto& item : sessions) {
                QJsonObject s = item.toObject();
                QString status = s["status"].toString();

                if (statusFilter != "all" && status != statusFilter) continue;

                int row = m_table->rowCount();
                m_table->insertRow(row);

                m_table->setItem(row, 0, new QTableWidgetItem(QString("SE-%1").arg(s["session_id"].toInt() + 50000)));

                if (m_role == "client") {
                    m_table->setItem(row, 1, new QTableWidgetItem(s["psychologist_name"].toString()));
                } else {
                    m_table->setItem(row, 1, new QTableWidgetItem(s["client_name"].toString()));
                }

                qint64 startTime = s["start_time"].toVariant().toLongLong();
                QString dtStr = QDateTime::fromSecsSinceEpoch(startTime).toString("dd.MM HH:mm");
                m_table->setItem(row, 2, new QTableWidgetItem(dtStr));

                QString format = s["format"].toString();
                m_table->setItem(row, 3, new QTableWidgetItem(format == "online" ? "Онлайн" : "Офлайн"));

                QString statusText = status;
                if (status == "scheduled") statusText = "Назначена";
                else if (status == "completed") statusText = "Завершена";
                else if (status == "cancelled") statusText = "Отменена";

                m_table->setItem(row, 4, new QTableWidgetItem(statusText));
            }
        }
    });
}

void SessionListDialog::onCancelSession() {
    int row = m_table->currentRow();
    if (row < 0) return;

    QString sessionIdStr = m_table->item(row, 0)->text();
    int sessionId = sessionIdStr.mid(3).toInt() - 50000;

    if (QMessageBox::question(this, "Отмена", "Вы уверены, что хотите отменить эту сессию?") != QMessageBox::Yes) {
        return;
    }

    QJsonObject request;
    request["action"] = "cancel_session";
    QJsonObject data;
    data["session_id"] = sessionId;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            QMessageBox::information(this, "Успех", "Сессия успешно отменена");
            loadSessions();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось отменить сессию");
        }
    });
}

