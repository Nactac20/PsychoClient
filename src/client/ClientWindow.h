#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QJsonArray>
#include "../common/DashboardCard.h"
#include <QVector>

class PsychologistListWidget;
class QTableWidget;
class QTextEdit;
class QLineEdit;
class QComboBox;

class ClientWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ClientWindow(int userId, const QString& userName, const QString& email, QWidget* parent = nullptr);

private slots:
    void onPsychologistSelected(int psychologistId);
    void onBookSlot();
    void onEditProfile();
    void onSessionFilterChanged();
    void onShowCalendar();
    void onShowSessions();
    void onFilterChanged();

private:
    void loadPsychologists();
    void loadMySessions();
    void updateDiaryCard();
    
    int m_userId;
    QString m_userName;
    QString m_email;
    int m_currentPsychologistId;
    QTimer* m_pollTimer;
    QJsonArray m_allSessions;
    
    PsychologistListWidget* m_psychologistList;
    QLineEdit* m_searchEdit;
    QComboBox* m_specFilter;
    QTextEdit* m_psychologistInfoDisplay;
    DashboardCard* m_nextSessionCard;
    DashboardCard* m_notificationCard;
    QPushButton* m_bookButton;
    
    void updateNotifications();
    
    QJsonArray m_psychologistsData;
};
