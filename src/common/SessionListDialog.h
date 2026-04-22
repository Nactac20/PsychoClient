#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class SessionListDialog : public QDialog {
    Q_OBJECT
public:
    explicit SessionListDialog(int userId, const QString& role, QWidget* parent = nullptr);

private slots:
    void loadSessions();
    void onCancelSession();

private:
    int m_userId;
    QString m_role;
    QTableWidget* m_table;
    QComboBox* m_filter;
    QPushButton* m_cancelBtn;
};

