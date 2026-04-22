#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class DashboardCard : public QWidget {
    Q_OBJECT
public:
    explicit DashboardCard(const QString& title, const QString& icon, QWidget* parent = nullptr);

    void setValue(const QString& value);
    void setTitle(const QString& title);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_valueLabel;
};

