#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QJsonArray>

class PsychologistListWidget : public QWidget {
    Q_OBJECT
public:
    explicit PsychologistListWidget(QWidget* parent = nullptr);
    void setPsychologists(const QJsonArray& psychologists);
signals:
    void psychologistSelected(int psychologistId);
private slots:
    void onItemClicked(const QModelIndex& index);
private:
    QTableWidget* m_table;
};
