#include "SlotCalendarWidget.h"
#include <QLabel>
#include <QVBoxLayout>

SlotCalendarWidget::SlotCalendarWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel("Выберите психолога и нажмите 'Забронировать'");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}

void SlotCalendarWidget::clear() {
}
