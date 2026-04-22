#pragma once
#include <QWidget>

class SlotCalendarWidget : public QWidget {
    Q_OBJECT
public:
    explicit SlotCalendarWidget(QWidget* parent = nullptr);
    void clear();
};
