#include "DashboardCard.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QLabel>

DashboardCard::DashboardCard(const QString& title, const QString& icon, QWidget* parent)
    : QWidget(parent) {

    setMinimumSize(180, 100);
    setMaximumHeight(120);
    setCursor(Qt::PointingHandCursor);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(5);

    m_iconLabel = new QLabel(icon);
    m_iconLabel->setStyleSheet("font-size: 20px; background: transparent;");

    m_titleLabel = new QLabel(title);
    m_titleLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: #718096; text-transform: uppercase; background: transparent;");

    m_valueLabel = new QLabel("...");
    m_valueLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2D3748; background: transparent;");
    m_valueLabel->setWordWrap(true);

    layout->addWidget(m_iconLabel);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_valueLabel);
    layout->addStretch();

    setStyleSheet(R"(
        DashboardCard {
            background-color: white;
            border: 1px solid #E2E8F0;
            border-radius: 12px;
        }
        DashboardCard:hover {
            border: 1px solid #3182CE;
            background-color: #F7FAFC;
        }
    )");
}

void DashboardCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}

void DashboardCard::setTitle(const QString& title) {
    m_titleLabel->setText(title);
}

void DashboardCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

