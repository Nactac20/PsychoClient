#pragma once
#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class BookSlotDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookSlotDialog(int psychologistId, QWidget* parent = nullptr);

    int getSelectedSlotId() const { return m_selectedSlotId; }
    bool isConfirmed() const { return m_confirmed; }

private slots:
    void onSlotSelected(int index);
    void onConfirm();
    void onCancel();

private:
    void loadSlots();

    int m_psychologistId;
    int m_selectedSlotId;
    bool m_confirmed;

    QComboBox* m_slotCombo;
    QPushButton* m_confirmButton;
    QPushButton* m_cancelButton;
    QLabel* m_statusLabel;
};
