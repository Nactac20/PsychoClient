#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QJsonArray>
#include <QVector>
#include <QTimer>

class ChatWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ChatWindow(int userId, const QString& userName, const QString& role, QWidget* parent = nullptr);

private slots:
    void onSendMessage();
    void onConversationSelected(int index);
    void loadMessages();
    void onDeleteChat();

private:
    void loadConversations();
    void addMessageToChat(int senderId, const QString& senderName, const QString& text, const QString& time);
    void sendRequest(const QString& action, const QJsonObject& data, std::function<void(const QJsonObject&)> callback);
    
    int m_userId;
    QString m_userName;
    QString m_role;
    
    QListWidget* m_conversationList;
    QTextEdit* m_chatDisplay;
    QLineEdit* m_messageInput;
    QPushButton* m_sendButton;
    QPushButton* m_deleteChatButton;
    
    QVector<int> m_sessionsData;
    int m_currentSessionId;
    int m_currentOtherUserId;
    QString m_currentOtherUserName;
    QTimer* m_pollTimer;
};
