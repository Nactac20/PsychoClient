#include "PsychologistWindow.h"
#include "../common/SessionCalendarDialog.h"
#include "../common/SessionListDialog.h"
#include "../auth/EditProfileDialog.h"
#include "../chat/ChatWindow.h"
#include "../common/NotificationListDialog.h"
#include "../network/NetworkManager.h"
#include "../network/ErrorHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QProcess>
#include <QApplication>
#include <QGroupBox>
#include <QDateTime>
#include <QPushButton>
#include <QJsonArray>
#include <QJsonObject>

PsychologistWindow::PsychologistWindow(int userId, const QString& userName, const QString& email, QWidget* parent)
    : QMainWindow(parent), m_userId(userId), m_userName(userName), m_email(email) {
    setWindowTitle(QString("PsychoClient - Психолог: %1").arg(userName));
    resize(900, 600);
    setupUI();
    loadSlots();
    loadMySessions();

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        loadSlots();
        loadMySessions();
        updateNotifications();
    });
    m_pollTimer->start(3000);
}

void PsychologistWindow::setProfileData(const QString& email, const QString& spec, const QString& edu, const QString& desc) {
    m_email = email;
    m_spec = spec;
    m_edu = edu;
    m_desc = desc;
}

void PsychologistWindow::setupUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QLabel* welcomeLabel = new QLabel(QString("Добро пожаловать, %1! (ID: PSY-%2)").arg(m_userName).arg(m_userId + 10000));
    welcomeLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(welcomeLabel);

    QHBoxLayout* dashLayout = new QHBoxLayout;
    m_nextSessionCard = new DashboardCard("Ближайшая запись", "📅");
    m_slotsCard = new DashboardCard("Свободных слотов", "⏳");
    m_notificationCard = new DashboardCard("Уведомления", "🔔");
    dashLayout->addWidget(m_nextSessionCard);
    dashLayout->addWidget(m_slotsCard);
    dashLayout->addWidget(m_notificationCard);
    mainLayout->addLayout(dashLayout);

    QGroupBox* addGroup = new QGroupBox("Добавить новый слот");
    QHBoxLayout* addLayout = new QHBoxLayout(addGroup);

    addLayout->addWidget(new QLabel("Дата и время:"));
    m_dateTimeEdit = new QDateTimeEdit;
    m_dateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(1));
    m_dateTimeEdit->setCalendarPopup(true);
    addLayout->addWidget(m_dateTimeEdit);

    addLayout->addWidget(new QLabel("Длительность:"));
    m_durationCombo = new QComboBox;
    m_durationCombo->addItem("30 мин", 30);
    m_durationCombo->addItem("45 мин", 45);
    m_durationCombo->addItem("60 мин", 60);
    m_durationCombo->addItem("90 мин", 90);
    addLayout->addWidget(m_durationCombo);

    addLayout->addWidget(new QLabel("Формат:"));
    m_formatCombo = new QComboBox;
    m_formatCombo->addItem("Онлайн", "online");
    m_formatCombo->addItem("Офлайн", "offline");
    addLayout->addWidget(m_formatCombo);

    m_addButton = new QPushButton("Добавить слот");
    addLayout->addWidget(m_addButton);

    mainLayout->addWidget(addGroup);

    QLabel* slotsLabel = new QLabel("Мои слоты:");
    slotsLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(slotsLabel);

    m_slotsTable = new QTableWidget;
    m_slotsTable->setColumnCount(4);
    m_slotsTable->setHorizontalHeaderLabels({"ID", "Дата и время", "Длительность", "Формат"});
    m_slotsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_slotsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_slotsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_slotsTable);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_deleteButton = new QPushButton("Удалить выбранный слот");
    m_deleteButton->setEnabled(false);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_deleteButton);
    mainLayout->addLayout(buttonLayout);
    

    QHBoxLayout* actionBtnLayout = new QHBoxLayout;
    QPushButton* sessionsButton = new QPushButton("Мои сессии");
    QPushButton* chatButton = new QPushButton("Чат с клиентами");
    QPushButton* profileButton = new QPushButton("Мой профиль");
    actionBtnLayout->addWidget(sessionsButton);
    actionBtnLayout->addWidget(chatButton);
    actionBtnLayout->addWidget(profileButton);
    mainLayout->addLayout(actionBtnLayout);

    connect(m_addButton, &QPushButton::clicked, this, &PsychologistWindow::onAddSlot);
    connect(m_deleteButton, &QPushButton::clicked, this, &PsychologistWindow::onDeleteSlot);
    connect(profileButton, &QPushButton::clicked, this, &PsychologistWindow::onEditProfile);
    connect(m_nextSessionCard, &DashboardCard::clicked, this, &PsychologistWindow::onShowCalendar);
    connect(m_slotsCard, &DashboardCard::clicked, this, &PsychologistWindow::onShowSlotsCalendar);
    connect(m_notificationCard, &DashboardCard::clicked, [this]() {
        NotificationListDialog dialog(m_userId, this);
        dialog.exec();
        updateNotifications();
    });
    connect(sessionsButton, &QPushButton::clicked, this, &PsychologistWindow::onShowSessions);

    connect(m_slotsTable, &QTableWidget::itemSelectionChanged, [this]() {
        bool selected = m_slotsTable->currentRow() >= 0;
        m_deleteButton->setEnabled(selected);
    });

    connect(chatButton, &QPushButton::clicked, [this]() {
        ChatWindow* chat = new ChatWindow(m_userId, m_userName, "psychologist");
        chat->show();
    });
}

void PsychologistWindow::loadSlots() {
    int selectedId = -1;
    int currentRow = m_slotsTable->currentRow();
    if (currentRow >= 0) {
        selectedId = m_slotsTable->item(currentRow, 0)->text().toInt();
    }

    QJsonObject request;
    request["action"] = "get_free_slots";
    QJsonObject data;
    data["psychologist_id"] = m_userId;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this, selectedId](const QJsonObject& response) {
        if (response["status"] == "success") {
            QJsonArray slotList = response["data"].toArray();
            m_slotsTable->setRowCount(0);

            int rowToSelect = -1;
            for (int i = 0; i < slotList.size(); ++i) {
                QJsonObject slot = slotList[i].toObject();
                int slotId = slot["slot_id"].toInt();
                qint64 startTime = slot["start_time"].toInteger();
                int duration = slot["duration"].toInt();
                QString format = slot["format"].toString();

                QDateTime dt = QDateTime::fromSecsSinceEpoch(startTime);
                addSlotToTable(slotId, dt.toString("dd.MM.yyyy HH:mm"), duration, format, false);

                if (slotId == selectedId) {
                    rowToSelect = i;
                }
            }
            m_slotsCard->setValue(QString("%1 слотов").arg(slotList.size()));

            if (rowToSelect >= 0) {
                m_slotsTable->selectRow(rowToSelect);
            }
        } else {
            QMessageBox::warning(this, "Ошибка", ErrorHelper::translate(response));
        }
    });
}

void PsychologistWindow::addSlotToTable(int slotId, const QString& dateTime, int duration, const QString& format, bool isBooked) {
    int row = m_slotsTable->rowCount();
    m_slotsTable->insertRow(row);
    m_slotsTable->setItem(row, 0, new QTableWidgetItem(QString::number(slotId)));
    m_slotsTable->setItem(row, 1, new QTableWidgetItem(dateTime));
    m_slotsTable->setItem(row, 2, new QTableWidgetItem(QString::number(duration) + " мин"));
    m_slotsTable->setItem(row, 3, new QTableWidgetItem(format == "online" ? "Онлайн" : "Офлайн"));
}

void PsychologistWindow::onAddSlot() {
    QDateTime dateTime = m_dateTimeEdit->dateTime();
    int duration = m_durationCombo->currentData().toInt();
    QString format = m_formatCombo->currentData().toString();

    QJsonObject request;
    request["action"] = "add_slot";
    QJsonObject data;
    data["psychologist_id"] = m_userId;
    data["start_time"] = static_cast<qint64>(dateTime.toSecsSinceEpoch());
    data["duration"] = duration;
    data["format"] = format;
    request["data"] = data;

    m_addButton->setEnabled(false);
    m_addButton->setText("Добавление...");

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        m_addButton->setEnabled(true);
        m_addButton->setText("Добавить слот");

        if (response["status"] == "success") {
            QMessageBox::information(this, "Успех", "Слот успешно добавлен!");
            loadSlots();
        } else {
            ErrorHelper::showError(this, response, "Добавление слота");
        }
    });
}

void PsychologistWindow::onDeleteSlot() {
    int row = m_slotsTable->currentRow();
    if (row < 0) return;

    int slotId = m_slotsTable->item(row, 0)->text().toInt();

    if (QMessageBox::question(this, "Удаление", "Удалить выбранный слот?") != QMessageBox::Yes) {
        return;
    }

    QJsonObject request;
    request["action"] = "delete_slot";
    QJsonObject data;
    data["slot_id"] = slotId;
    data["psychologist_id"] = m_userId;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            QMessageBox::information(this, "Успех", "Слот удалён");
            loadSlots();
        } else {
            ErrorHelper::showError(this, response, "Удаление слота");
        }
    });
}

void PsychologistWindow::onEditProfile() {
    EditProfileDialog dialog(m_userId, "psychologist", this);
    dialog.setProfileData(m_userName, m_email, m_spec, m_edu, m_desc);

    connect(&dialog, &EditProfileDialog::accountDeleted, this, []() {
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        qApp->quit();
    });

    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString email = dialog.getEmail();
        QString password = dialog.getPassword();
        QString spec = dialog.getSpec();
        QString edu = dialog.getEdu();
        QString desc = dialog.getDesc();

        QJsonObject request;
        request["action"] = "update_user";
        QJsonObject data;
        data["user_id"] = m_userId;
        data["name"] = name;
        data["email"] = email;
        if (!password.isEmpty()) data["password"] = password;
        data["specialization"] = spec;
        data["education"] = edu;
        data["description"] = desc;
        request["data"] = data;

        NetworkManager::instance().sendRequest(request, [this, name, email, spec, edu, desc](const QJsonObject& response) {
            if (response["status"] == "success") {
                m_userName = name;
                m_email = email;
                m_spec = spec;
                m_edu = edu;
                m_desc = desc;
                setWindowTitle(QString("PsychoClient - Психолог: %1").arg(m_userName));
                QMessageBox::information(this, "Успех", "Профиль обновлен");
            } else {
                ErrorHelper::showError(this, response, "Ошибка обновления");
            }
        });
    }
}

void PsychologistWindow::loadMySessions() {
    QJsonObject request;
    request["action"] = "get_my_sessions";
    QJsonObject data;
    data["psychologist_id"] = m_userId;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            m_allSessions = response["data"].toArray();

            QJsonObject nextSession;
            qint64 minTime = std::numeric_limits<qint64>::max();

            for (const auto& item : m_allSessions) {
                QJsonObject s = item.toObject();
                QString status = s["status"].toString();

                if (status == "scheduled") {
                    qint64 startTime = s["start_time"].toVariant().toLongLong();
                    if (startTime > QDateTime::currentSecsSinceEpoch() && startTime < minTime) {
                        minTime = startTime;
                        nextSession = s;
                    }
                }
            }

            if (minTime != std::numeric_limits<qint64>::max()) {
                QString dtStr = QDateTime::fromSecsSinceEpoch(minTime).toString("dd.MM HH:mm");
                m_nextSessionCard->setValue(QString("%1\n%2").arg(nextSession["client_name"].toString(), dtStr));
            } else {
                m_nextSessionCard->setValue("Нет записей");
            }
        }
    });
}

void PsychologistWindow::onShowSessions() {
    SessionListDialog dialog(m_userId, "psychologist", this);
    dialog.exec();
    loadMySessions();
}

void PsychologistWindow::onShowCalendar() {
    SessionCalendarDialog dialog(m_allSessions, "psychologist", this);
    dialog.exec();
}

void PsychologistWindow::onShowSlotsCalendar() {
    QJsonObject request;
    request["action"] = "get_free_slots";
    QJsonObject data;
    data["psychologist_id"] = m_userId;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            QJsonArray slotsList = response["data"].toArray();
            QJsonArray mappedSessions;
            for (const auto& item : slotsList) {
                QJsonObject slot = item.toObject();
                QJsonObject s;
                s["start_time"] = slot["start_time"];
                s["status"] = "scheduled";
                s["client_name"] = "Свободный слот (" + slot["format"].toString() + ")";
                mappedSessions.append(s);
            }
            SessionCalendarDialog dialog(mappedSessions, "psychologist", this);
            dialog.setWindowTitle("Календарь свободных слотов");
            dialog.exec();
        }
    });
}

void PsychologistWindow::updateNotifications() {
    QJsonObject request;
    request["action"] = "get_notifications";
    QJsonObject data;
    data["user_id"] = m_userId;
    data["only_unread"] = true;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            int unreadCount = response["data"].toArray().size();
            if (unreadCount > 0) {
                m_notificationCard->setValue(QString("Новых: %1").arg(unreadCount));
                m_notificationCard->setStyleSheet("DashboardCard { border: 2px solid #3182CE; background-color: #EBF8FF; }");
            } else {
                m_notificationCard->setValue("Нет новых");
                m_notificationCard->setStyleSheet("");
            }
        }
    });
}
