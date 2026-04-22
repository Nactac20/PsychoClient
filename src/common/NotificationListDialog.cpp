#include "NotificationListDialog.h"
#include "../network/NetworkManager.h"
#include "../network/ErrorHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

NotificationListDialog::NotificationListDialog(int userId, QWidget* parent)
    : QDialog(parent), m_userId(userId) {

    setWindowTitle("Уведомления");
    setMinimumSize(500, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_list = new QListWidget;
    m_list->setWordWrap(true);
    m_list->setSpacing(5);
    mainLayout->addWidget(m_list);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    m_markReadBtn = new QPushButton("Прочитано");
    m_deleteBtn = new QPushButton("Удалить");
    QPushButton* closeBtn = new QPushButton("Закрыть");

    m_markReadBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    btnLayout->addWidget(m_markReadBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_markReadBtn, &QPushButton::clicked, this, &NotificationListDialog::markAsRead);
    connect(m_deleteBtn, &QPushButton::clicked, this, &NotificationListDialog::deleteNotification);

    connect(m_list, &QListWidget::itemSelectionChanged, [this]() {
        bool selected = m_list->currentRow() >= 0;
        m_markReadBtn->setEnabled(selected);
        m_deleteBtn->setEnabled(selected);
    });

    loadNotifications();
}

void NotificationListDialog::loadNotifications() {
    QJsonObject request;
    request["action"] = "get_notifications";
    QJsonObject data;
    data["user_id"] = m_userId;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            m_list->clear();
            QJsonArray notifications = response["data"].toArray();

            if (notifications.isEmpty()) {
                m_list->addItem("Уведомлений пока нет");
                m_list->item(0)->setFlags(Qt::NoItemFlags);
                return;
            }

            for (int i = 0; i < notifications.size(); ++i) {
                QJsonObject n = notifications[i].toObject();
                int id = n["id"].toInt();
                QString text = n["text"].toString();
                QString type = n["type"].toString();
                bool isRead = n["is_read"].toBool();
                qint64 time = n["created_at"].toVariant().toLongLong();

                QString dateStr = QDateTime::fromSecsSinceEpoch(time).toString("dd.MM HH:mm");

                QListWidgetItem* item = new QListWidgetItem;
                item->setData(Qt::UserRole, id);

                QString display = QString("[%1] %2").arg(dateStr, text);
                item->setText(display);

                if (!isRead) {
                    QFont font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                    item->setBackground(QBrush(QColor("#EBF8FF")));
                }

                if (type == "warning") {
                    item->setForeground(QBrush(QColor("#C53030")));
                }

                m_list->addItem(item);
            }
        }
    });
}

void NotificationListDialog::markAsRead() {
    QListWidgetItem* item = m_list->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    if (id <= 0) return;

    QJsonObject request;
    request["action"] = "mark_notification_read";
    QJsonObject data;
    data["notification_id"] = id;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            loadNotifications();
        }
    });
}

void NotificationListDialog::deleteNotification() {
    QListWidgetItem* item = m_list->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    if (id <= 0) return;

    QJsonObject request;
    request["action"] = "delete_notification";
    QJsonObject data;
    data["notification_id"] = id;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            loadNotifications();
        }
    });
}

