#include "PsychologistListWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QJsonObject>

PsychologistListWidget::PsychologistListWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QLabel* title = new QLabel("Психологи");
    title->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(title);
    
    m_table = new QTableWidget;
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"ID", "Имя", "Специализация"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(45);
    layout->addWidget(m_table);
    
    connect(m_table, &QTableWidget::clicked, this, &PsychologistListWidget::onItemClicked);
}

void PsychologistListWidget::setPsychologists(const QJsonArray& psychologists) {
    m_table->setRowCount(0);
    
    for (int i = 0; i < psychologists.size(); ++i) {
        QJsonObject obj = psychologists[i].toObject();
        int row = m_table->rowCount();
        m_table->insertRow(row);
        
        m_table->setItem(row, 0, new QTableWidgetItem(QString("PSY-%1").arg(obj["id"].toInt() + 10000)));
        m_table->setItem(row, 1, new QTableWidgetItem(obj["name"].toString()));
        m_table->setItem(row, 2, new QTableWidgetItem(obj["specialization"].toString()));
    }
}

void PsychologistListWidget::onItemClicked(const QModelIndex& index) {
    int row = index.row();
    QString idStr = m_table->item(row, 0)->text();
    idStr.remove("PSY-");
    int psychologistId = idStr.toInt() - 10000;
    emit psychologistSelected(psychologistId);
}
