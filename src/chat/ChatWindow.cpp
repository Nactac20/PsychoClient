#include "ChatWindow.h"
#include "../network/NetworkManager.h"
#include "../network/ErrorHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QListWidgetItem>

ChatWindow::ChatWindow(int userId, const QString& userName, const QString& role, QWidget* parent)
    : QMainWindow(parent)
    , m_userId(userId)
    , m_userName(userName)
    , m_role(role)
    , m_currentSessionId(-1)
    , m_currentOtherUserId(-1) {
    
    setWindowTitle(QString("PsychoClient - Чат (%1)").arg(userName));
    resize(800, 600);
    
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    
    QWidget* leftPanel = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    
    QLabel* convLabel = new QLabel("Диалоги");
    convLabel->setStyleSheet("font-weight: bold;");
    leftLayout->addWidget(convLabel);
    
    m_conversationList = new QListWidget;
    leftLayout->addWidget(m_conversationList);
    
    m_deleteChatButton = new QPushButton("Удалить чат");
    m_deleteChatButton->setEnabled(false);
    leftLayout->addWidget(m_deleteChatButton);
    
    mainLayout->addWidget(leftPanel, 1);
    
    QWidget* rightPanel = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    
    m_chatDisplay = new QTextEdit;
    m_chatDisplay->setReadOnly(true);
    rightLayout->addWidget(m_chatDisplay);
    
    QHBoxLayout* inputLayout = new QHBoxLayout;
    m_messageInput = new QLineEdit;
    m_messageInput->setPlaceholderText("Введите сообщение...");
    m_sendButton = new QPushButton("Отправить");
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    rightLayout->addLayout(inputLayout);
    
    mainLayout->addWidget(rightPanel, 2);
    
    connect(m_conversationList, &QListWidget::currentRowChanged, this, &ChatWindow::onConversationSelected);
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::onSendMessage);
    connect(m_deleteChatButton, &QPushButton::clicked, this, &ChatWindow::onDeleteChat);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendMessage);
    
    loadConversations();
    
    m_pollTimer = new QTimer(this);
    m_pollTimer->start(2000);
    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        loadConversations();
        if (m_currentSessionId > 0) {
            loadMessages();
        }
    });
}

void ChatWindow::loadConversations() {
    int selectedConvId = -1;
    int currentRow = m_conversationList->currentRow();
    if (currentRow >= 0 && currentRow < m_sessionsData.size()) {
        selectedConvId = m_sessionsData[currentRow];
    }
    
    QJsonObject data;
    data["user_id"] = m_userId;
    
    sendRequest("get_conversations", data, [this, selectedConvId](const QJsonObject& response) {
        if (response["status"] == "success") {
            QJsonArray convs = response["data"].toArray();
            
            bool changed = false;
            if (convs.size() != m_sessionsData.size()) {
                changed = true;
            } else {
                for (int i = 0; i < convs.size(); ++i) {
                    if (convs[i].toObject()["conversation_id"].toInt() != m_sessionsData[i]) {
                        changed = true;
                        break;
                    }
                }
            }
            
            if (!changed) return; 
            
            int rowToSelect = -1;
            m_conversationList->clear(); 
            m_sessionsData.clear(); 
            
            for (const auto& item : convs) {
                QJsonObject obj = item.toObject();
                QString name = obj["other_user_name"].toString();
                int convId = obj["conversation_id"].toInt();
                
                int row = m_conversationList->count();
                m_conversationList->addItem(name);
                m_sessionsData.append(convId);
                
                if (convId == selectedConvId) {
                    rowToSelect = row;
                }
            }
            
            if (rowToSelect >= 0) {
                m_conversationList->setCurrentRow(rowToSelect);
            }
        }
    });
}

void ChatWindow::onConversationSelected(int index) {
    if (index < 0 || index >= m_sessionsData.size()) {
        m_currentSessionId = -1;
        m_deleteChatButton->setEnabled(false);
        m_chatDisplay->clear();
        return;
    }
    
    m_currentSessionId = m_sessionsData[index]; 
    m_deleteChatButton->setEnabled(true);
    setWindowTitle(QString("Чат с %1").arg(m_conversationList->item(index)->text()));
    loadMessages();
}

void ChatWindow::onDeleteChat() {
    if (m_currentSessionId <= 0) return;
    
    if (QMessageBox::question(this, "Удаление", "Удалить этот чат и сообщения?") != QMessageBox::Yes) {
        return;
    }
    
    QJsonObject data;
    data["conversation_id"] = m_currentSessionId;
    
    sendRequest("delete_conversation", data, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            m_currentSessionId = -1;
            m_chatDisplay->clear();
            loadConversations();
        } else {
            QMessageBox::warning(this, "Ошибка", ErrorHelper::translate(response));
        }
    });
}

void ChatWindow::loadMessages() {
    QJsonObject data;
    data["conversation_id"] = m_currentSessionId;
    
    sendRequest("get_messages", data, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            QJsonArray messages = response["data"].toArray();
            m_chatDisplay->clear();
            
            for (const auto& item : messages) {
                QJsonObject obj = item.toObject();
                int senderId = obj["sender_id"].toInt();
                QString senderName = obj["sender_name"].toString();
                QString text = obj["text"].toString();
                qint64 timestamp = obj["timestamp"].toInt();
                
                QString timeStr = QDateTime::fromSecsSinceEpoch(timestamp).toString("hh:mm");
                QString align = (senderId == m_userId) ? "right" : "left";
                QString color = (senderId == m_userId) ? "#DCF8C6" : "#FFFFFF";
                
                m_chatDisplay->append(QString(
                    "<div style='text-align: %1; margin: 5px;'>"
                    "<div style='display: inline-block; background-color: %2; padding: 8px; border-radius: 10px; max-width: 70%%;'>"
                    "<b>%3</b> <small style='color: gray;'>%4</small><br>%5"
                    "</div></div>"
                ).arg(align).arg(color).arg(senderName).arg(timeStr).arg(text.toHtmlEscaped()));
            }
        }
    });
}

void ChatWindow::onSendMessage() {
    QString text = m_messageInput->text().trimmed();
    if (text.isEmpty()) return;
    
    sendRequest("send_message", {
        {"conversation_id", m_currentSessionId},
        {"sender_id", m_userId},
        {"text", text}
    }, [this](const QJsonObject& response) {
        if (response["status"] == "success") {
            m_messageInput->clear();
            loadMessages();
        } else {
            QMessageBox::warning(this, "Ошибка", ErrorHelper::translate(response));
        }
    });
}

void ChatWindow::sendRequest(const QString& action, const QJsonObject& data, std::function<void(const QJsonObject&)> callback) {
    QJsonObject request;
    request["action"] = action;
    request["data"] = data;
    NetworkManager::instance().sendRequest(request, callback);
}
