#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QSpinBox>
#include <QTimer>
#include <QJsonArray>
#include "../common/DashboardCard.h"

class PsychologistWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit PsychologistWindow(int userId, const QString& userName, const QString& email, QWidget* parent = nullptr);

    void setProfileData(const QString& email, const QString& spec, const QString& edu, const QString& desc);

private slots:
    void onAddSlot();
    void onDeleteSlot();
    void onEditProfile();
    void loadSlots();
    void loadMySessions();
    void onShowCalendar();
    void onShowSessions();
    void onShowSlotsCalendar();

private:
    void setupUI();
    void addSlotToTable(int slotId, const QString& dateTime, int duration, const QString& format, bool isBooked);

    int m_userId;
    QString m_userName;
    QString m_email;
    QString m_spec, m_edu, m_desc;


    QTableWidget* m_slotsTable;
    QDateTimeEdit* m_dateTimeEdit;
    QComboBox* m_durationCombo;
    QComboBox* m_formatCombo;
    QPushButton* m_addButton;
    QPushButton* m_deleteButton;
    DashboardCard* m_nextSessionCard;
    DashboardCard* m_slotsCard;
    DashboardCard* m_notificationCard;
    QTimer* m_pollTimer;

    void updateNotifications();
    QJsonArray m_allSessions;
};
