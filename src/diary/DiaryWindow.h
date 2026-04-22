#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>

class DiaryWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit DiaryWindow(int userId, const QString& userName, QWidget* parent = nullptr);

private slots:
    void onAddEntry();
    void onEntrySelected(int index);
    void onDeleteEntry();

private:
    void setupUI();
    void loadEntries();
    void saveEntries();

    int m_userId;
    QString m_userName;

    QListWidget* m_entryList;
    QTextEdit* m_entryText;
    QLineEdit* m_titleInput;
    QDateEdit* m_dateEdit;
    QPushButton* m_addButton;
    QPushButton* m_deleteButton;
    QPushButton* m_saveButton;

    struct DiaryEntry {
        QString title;
        QString text;
        QString date;
    };
    QVector<DiaryEntry> m_entries;
    int m_currentIndex = -1;
};

