#include "BookSlotDialog.h"
#include "../network/NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

BookSlotDialog::BookSlotDialog(int psychologistId, QWidget* parent)
    : QDialog(parent)
    , m_psychologistId(psychologistId)
    , m_selectedSlotId(-1)
    , m_confirmed(false) {

    setWindowTitle(QString("Бронирование психолога ID: %1").arg(psychologistId));
    setModal(true);
    resize(400, 250);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel(QString("Психолог ID: %1\nВыберите время для консультации:").arg(psychologistId));
    layout->addWidget(label);

    m_slotCombo = new QComboBox;
    layout->addWidget(m_slotCombo);

    m_statusLabel = new QLabel;
    m_statusLabel->setVisible(false);
    layout->addWidget(m_statusLabel);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_confirmButton = new QPushButton("Подтвердить");
    m_cancelButton = new QPushButton("Отмена");
    m_confirmButton->setEnabled(false);
    buttonLayout->addWidget(m_confirmButton);
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);

    connect(m_slotCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BookSlotDialog::onSlotSelected);
    connect(m_confirmButton, &QPushButton::clicked, this, &BookSlotDialog::onConfirm);
    connect(m_cancelButton, &QPushButton::clicked, this, &BookSlotDialog::onCancel);

    loadSlots();
}

void BookSlotDialog::loadSlots() {
    m_statusLabel->setText("Загрузка слотов...");
    m_statusLabel->setVisible(true);

    QJsonObject request;
    request["action"] = "get_free_slots";
    QJsonObject data;
    data["psychologist_id"] = m_psychologistId;
    request["data"] = data;

    NetworkManager::instance().sendRequest(request, [this](const QJsonObject& response) {
        m_statusLabel->setVisible(false);
        m_slotCombo->clear();

        if (response["status"] == "success") {
            QJsonArray slotList = response["data"].toArray();

            if (slotList.isEmpty()) {
                m_slotCombo->addItem("Нет свободных слотов");
                m_confirmButton->setEnabled(false);
                return;
            }

            for (int i = 0; i < slotList.size(); ++i) {
                QJsonObject slot = slotList[i].toObject();
                int slotId = slot["slot_id"].toInt();
                qint64 startTime = slot["start_time"].toInteger();
                int duration = slot["duration"].toInt();
                QString format = slot["format"].toString();

                QDateTime dt = QDateTime::fromSecsSinceEpoch(startTime);
                QString displayText = QString("%1 — %2 мин (%3)")
                    .arg(dt.toString("dd.MM.yyyy HH:mm"))
                    .arg(duration)
                    .arg(format == "online" ? "Онлайн" : "Офлайн");

                m_slotCombo->addItem(displayText, slotId);
            }

            m_selectedSlotId = m_slotCombo->itemData(0).toInt();
            m_confirmButton->setEnabled(true);
        } else {
            m_slotCombo->addItem("Ошибка загрузки слотов");
            m_confirmButton->setEnabled(false);
        }
    });
}

void BookSlotDialog::onSlotSelected(int index) {
    if (index >= 0) {
        m_selectedSlotId = m_slotCombo->itemData(index).toInt();
    }
}

void BookSlotDialog::onConfirm() {
    m_confirmed = true;
    accept();
}

void BookSlotDialog::onCancel() {
    reject();
}
