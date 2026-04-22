#pragma once
#include <QDialog>
#include <QListWidget>

class NotificationListDialog : public QDialog {
    Q_OBJECT
public:
    explicit NotificationListDialog(int userId, QWidget* parent = nullptr);

private slots:
    void loadNotifications();
    void markAsRead();
    void deleteNotification();

private:
    int m_userId;
    QListWidget* m_list;
    QPushButton* m_markReadBtn;
    QPushButton* m_deleteBtn;
};

