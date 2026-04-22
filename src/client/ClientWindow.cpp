#include "ClientWindow.h"
#include "../common/SessionCalendarDialog.h"
#include "../common/SessionListDialog.h"
#include "PsychologistListWidget.h"
#include "BookSlotDialog.h"
#include "../auth/EditProfileDialog.h"
#include "../chat/ChatWindow.h"
#include "../diary/DiaryWindow.h"
#include "../common/NotificationListDialog.h"
#include "../network/NetworkManager.h"
#include "../network/ErrorHelper.h"
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextEdit>
#include <QTableWidget>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QProcess>
#include <QApplication>

ClientWindow::ClientWindow(int userId, const QString& userName, const QString& email, QWidget* parent)
    : QMainWindow(parent)
    , m_userId(userId)
    , m_userName(userName)
    , m_email(email)
    , m_currentPsychologistId(-1) {
    
    setWindowTitle(QString("PsychoClient - Клиент: %1").arg(userName));
    resize(800, 600);
    
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    QLabel* welcomeLabel = new QLabel(QString("Добро пожаловать, %1! (ID: CLI-%2)").arg(userName).arg(userId + 20000));
    welcomeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(welcomeLabel);

    QHBoxLayout* dashLayout = new QHBoxLayout;
    m_nextSessionCard = new DashboardCard("Ближайшая сессия", "📅");
    m_notificationCard = new DashboardCard("Уведомления", "🔔");
    dashLayout->addWidget(m_nextSessionCard);
    dashLayout->addWidget(m_notificationCard);
    mainLayout->addLayout(dashLayout);

    QHBoxLayout* filterLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Поиск по имени...");
    m_specFilter = new QComboBox;
    m_specFilter->addItem("Все специализации");
    
    filterLayout->addWidget(new QLabel("Поиск:"));
    filterLayout->addWidget(m_searchEdit);
    filterLayout->addWidget(new QLabel("Специализация:"));
    filterLayout->addWidget(m_specFilter);
    mainLayout->addLayout(filterLayout);
    
    m_psychologistList = new PsychologistListWidget;
    mainLayout->addWidget(m_psychologistList);
    
    m_psychologistInfoDisplay = new QTextEdit;
    m_psychologistInfoDisplay->setReadOnly(true);
    m_psychologistInfoDisplay->setPlaceholderText("Информация о выбранном психологе появится здесь...");
    mainLayout->addWidget(m_psychologistInfoDisplay);
    
    m_bookButton = new QPushButton("Забронировать выбранного психолога");
    m_bookButton->setEnabled(false);
    mainLayout->addWidget(m_bookButton);
    
    
    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* sessionsButton = new QPushButton("Мои сессии");
    QPushButton* chatButton = new QPushButton("Чат с психологом");
    QPushButton* diaryButton = new QPushButton("Мой дневник");
    QPushButton* profileButton = new QPushButton("Профиль");
    btnLayout->addWidget(sessionsButton);
    btnLayout->addWidget(chatButton);
    btnLayout->addWidget(diaryButton);
    btnLayout->addWidget(profileButton);
    mainLayout->addLayout(btnLayout);
    
    mainLayout->addStretch();
    
    connect(m_psychologistList, &PsychologistListWidget::psychologistSelected, this, &ClientWindow::onPsychologistSelected);
    connect(m_bookButton, &QPushButton::clicked, this, &ClientWindow::onBookSlot);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ClientWindow::onFilterChanged);
    connect(m_specFilter, &QComboBox::currentTextChanged, this, &ClientWindow::onFilterChanged);
    connect(m_nextSessionCard, &DashboardCard::clicked, this, &ClientWindow::onShowCalendar);
    connect(sessionsButton, &QPushButton::clicked, this, &ClientWindow::onShowSessions);
    connect(chatButton, &QPushButton::clicked, [this]() {
        ChatWindow* chat = new ChatWindow(m_userId, m_userName, "client");
        chat->show();
    });
    connect(diaryButton, &QPushButton::clicked, [this]() {
        DiaryWindow* diary = new DiaryWindow(m_userId, m_userName);
        diary->show();
    });
    connect(m_notificationCard, &DashboardCard::clicked, [this]() {
        NotificationListDialog dialog(m_userId, this);
        dialog.exec();
        updateNotifications();
    });
    connect(profileButton, &QPushButton::clicked, this, &ClientWindow::onEditProfile);
    
    loadPsychologists();
    loadMySessions();

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        loadPsychologists();
        loadMySessions();
        updateNotifications();
    });
    m_pollTimer->start(3000);
}

void ClientWindow::loadPsychologists() {
    int selectedId = m_currentPsychologistId;

    QJsonObject request;
    request["action"] = "get_psychologists";
    request["data"] = QJsonObject();
    
    NetworkManager::instance().sendRequest(request, [this, selectedId](const QJsonObject& response) {
        if (response["status"] == "success") {
            m_psychologistsData = response["data"].toArray();
            onFilterChanged();
            
            QString currentSpec = m_specFilter->currentText();
            m_specFilter->blockSignals(true);
            m_specFilter->clear();
            m_specFilter->addItem("Все специализации");
            QStringList specs;
            for (int i = 0; i < m_psychologistsData.size(); ++i) {
                QString spec = m_psychologistsData[i].toObject()["specialization"].toString();
                if (!spec.isEmpty() && !specs.contains(spec)) {
                    specs.append(spec);
                }
            }
            specs.sort();
            m_specFilter->addItems(specs);
            int idx = m_specFilter->findText(currentSpec);
            if (idx >= 0) m_specFilter->setCurrentIndex(idx);
            m_specFilter->blockSignals(false);

            if (selectedId != -1) {
                onPsychologistSelected(selectedId);
            }
        } else {
            ErrorHelper::showError(this, response, "Загрузка");
        }
    });
}

void ClientWindow::onFilterChanged() {
    QString searchText = m_searchEdit->text().toLower();
    QString specText = m_specFilter->currentText();
    
    QJsonArray filtered;
    for (int i = 0; i < m_psychologistsData.size(); ++i) {
        QJsonObject obj = m_psychologistsData[i].toObject();
        QString name = obj["name"].toString().toLower();
        QString spec = obj["specialization"].toString();
        
        bool matchesSearch = name.contains(searchText);
        bool matchesSpec = (specText == "Все специализации" || spec == specText);
        
        if (matchesSearch && matchesSpec) {
            filtered.append(obj);
        }
    }
    m_psychologistList->setPsychologists(filtered);
}

void ClientWindow::onPsychologistSelected(int psychologistId) {
    m_currentPsychologistId = psychologistId;
    m_bookButton->setEnabled(true);
    m_bookButton->setText(QString("Забронировать психолога ID: PSY-%1").arg(psychologistId + 10000));
    
    for (int i = 0; i < m_psychologistsData.size(); ++i) {
        QJsonObject obj = m_psychologistsData[i].toObject();
        if (obj["id"].toInt() == psychologistId) {
            QString info = QString("<b>Имя:</b> %1<br>"
                                   "<b>Специализация:</b> %2<br>"
                                   "<b>Образование:</b> %3<br>"
                                   "<b>Описание:</b> %4")
                               .arg(obj["name"].toString())
                               .arg(obj["specialization"].toString())
                               .arg(obj["education"].toString())
                               .arg(obj["description"].toString());
            m_psychologistInfoDisplay->setHtml(info);
            break;
        }
    }
}

void ClientWindow::onBookSlot() {
    if (m_currentPsychologistId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите психолога для бронирования");
        return;
    }
    
    BookSlotDialog dialog(m_currentPsychologistId, this);
    if (dialog.exec() == QDialog::Accepted && dialog.isConfirmed()) {
        int slotId = dialog.getSelectedSlotId();
        
        QJsonObject request;
        request["action"] = "book_slot";
        QJsonObject data;
        data["slot_id"] = slotId;
        data["client_id"] = m_userId;
        request["data"] = data;
        
        m_bookButton->setEnabled(false);
        m_bookButton->setText("Бронирование...");
        
        NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
            m_bookButton->setEnabled(true);
            m_bookButton->setText("Забронировать выбранного психолога");
            
            if (response["status"] == "success") {
                int sessionId = response["data"].toObject()["session_id"].toInt();
                QMessageBox::information(this, "Успех", QString("Слот успешно забронирован!\nID сеанса: SE-%1").arg(sessionId + 50000));
            } else {
                ErrorHelper::showError(this, response, "Бронирование");
            }
        });
    }
}

void ClientWindow::onEditProfile() {
    EditProfileDialog dialog(m_userId, "client", this);
    dialog.setProfileData(m_userName, m_email);
    
    connect(&dialog, &EditProfileDialog::accountDeleted, this, []() {
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        qApp->quit();
    });
    
    if (dialog.exec() == QDialog::Accepted) {
        QString newName = dialog.getName();
        QString newEmail = dialog.getEmail();
        QString newPassword = dialog.getPassword();
        
        QJsonObject request;
        request["action"] = "update_user";
        QJsonObject data;
        data["user_id"] = m_userId;
        data["name"] = newName;
        data["email"] = newEmail;
        if (!newPassword.isEmpty()) data["password"] = newPassword;
        request["data"] = data;
        
        NetworkManager::instance().sendRequest(request, [this, newName, newEmail](const QJsonObject& response) {
            if (response["status"] == "success") {
                m_userName = newName;
                m_email = newEmail;
                setWindowTitle(QString("PsychoClient - Клиент: %1").arg(m_userName));
                QMessageBox::information(this, "Успех", "Профиль обновлен");
            } else {
                ErrorHelper::showError(this, response, "Ошибка обновления");
            }
        });
    }
}

void ClientWindow::onSessionFilterChanged() {
    loadMySessions();
}

void ClientWindow::updateNotifications() {
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

void ClientWindow::loadMySessions() {
    QJsonObject request;
    request["action"] = "get_my_sessions";
    QJsonObject data;
    data["client_id"] = m_userId;
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
                m_nextSessionCard->setValue(QString("%1\n%2").arg(nextSession["psychologist_name"].toString(), dtStr));
            } else {
                m_nextSessionCard->setValue("Нет записей");
            }
        }
    });
}

void ClientWindow::onShowSessions() {
    SessionListDialog dialog(m_userId, "client", this);
    dialog.exec();
    loadMySessions();
}

void ClientWindow::onShowCalendar() {
    SessionCalendarDialog dialog(m_allSessions, "client", this);
    dialog.exec();
}
