#include "DiaryWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QSplitter>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

DiaryWindow::DiaryWindow(int userId, const QString& userName, QWidget* parent)
    : QMainWindow(parent)
    , m_userId(userId)
    , m_userName(userName) {
    setWindowTitle(QString("Дневник — %1").arg(userName));
    resize(800, 600);
    setupUI();
    loadEntries();
}

void DiaryWindow::setupUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    QWidget* leftPanel = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* entriesLabel = new QLabel("Записи:");
    entriesLabel->setStyleSheet("font-weight: bold;");
    leftLayout->addWidget(entriesLabel);

    m_entryList = new QListWidget;
    leftLayout->addWidget(m_entryList);

    m_deleteButton = new QPushButton("Удалить запись");
    m_deleteButton->setEnabled(false);
    leftLayout->addWidget(m_deleteButton);

    splitter->addWidget(leftPanel);

    QWidget* rightPanel = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* newEntryGroup = new QGroupBox("Новая запись");
    QVBoxLayout* newEntryLayout = new QVBoxLayout(newEntryGroup);

    QHBoxLayout* titleRow = new QHBoxLayout;
    titleRow->addWidget(new QLabel("Заголовок:"));
    m_titleInput = new QLineEdit;
    m_titleInput->setPlaceholderText("Введите заголовок записи...");
    titleRow->addWidget(m_titleInput);

    titleRow->addWidget(new QLabel("Дата:"));
    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    titleRow->addWidget(m_dateEdit);

    m_addButton = new QPushButton("Добавить");
    titleRow->addWidget(m_addButton);
    newEntryLayout->addLayout(titleRow);

    rightLayout->addWidget(newEntryGroup);

    QLabel* textLabel = new QLabel("Текст записи:");
    textLabel->setStyleSheet("font-weight: bold;");
    rightLayout->addWidget(textLabel);

    m_entryText = new QTextEdit;
    m_entryText->setPlaceholderText("Выберите запись из списка или создайте новую...");
    rightLayout->addWidget(m_entryText);

    m_saveButton = new QPushButton("Сохранить изменения");
    m_saveButton->setEnabled(false);
    rightLayout->addWidget(m_saveButton);

    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter);

    connect(m_entryList, &QListWidget::currentRowChanged, this, &DiaryWindow::onEntrySelected);
    connect(m_addButton, &QPushButton::clicked, this, &DiaryWindow::onAddEntry);
    connect(m_deleteButton, &QPushButton::clicked, this, &DiaryWindow::onDeleteEntry);
    connect(m_saveButton, &QPushButton::clicked, [this]() {
        if (m_currentIndex >= 0 && m_currentIndex < m_entries.size()) {
            m_entries[m_currentIndex].text = m_entryText->toPlainText();
            saveEntries();
            QMessageBox::information(this, "Сохранено", "Запись обновлена");
        }
    });
}

void DiaryWindow::onAddEntry() {
    QString title = m_titleInput->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите заголовок записи");
        return;
    }

    DiaryEntry entry;
    entry.title = title;
    entry.date = m_dateEdit->date().toString("dd.MM.yyyy");
    entry.text = "";

    m_entries.prepend(entry);
    m_entryList->insertItem(0, QString("[%1] %2").arg(entry.date, entry.title));
    m_entryList->setCurrentRow(0);
    m_titleInput->clear();

    saveEntries();
}

void DiaryWindow::onEntrySelected(int index) {
    m_currentIndex = index;

    if (index >= 0 && index < m_entries.size()) {
        m_entryText->setText(m_entries[index].text);
        m_entryText->setEnabled(true);
        m_saveButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
    } else {
        m_entryText->clear();
        m_entryText->setEnabled(false);
        m_saveButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
    }
}

void DiaryWindow::onDeleteEntry() {
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) return;

    if (QMessageBox::question(this, "Удаление", "Удалить эту запись?") != QMessageBox::Yes) return;

    m_entries.removeAt(m_currentIndex);
    delete m_entryList->takeItem(m_currentIndex);
    m_currentIndex = -1;
    m_entryText->clear();
    m_entryText->setEnabled(false);
    m_saveButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    saveEntries();
}

void DiaryWindow::loadEntries() {
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dirPath);
    QString filePath = dirPath + QString("/diary_%1.json").arg(m_userId);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonArray array = doc.array();
    m_entries.clear();
    m_entryList->clear();

    for (int i = 0; i < array.size(); ++i) {
        QJsonObject obj = array[i].toObject();
        DiaryEntry entry;
        entry.title = obj["title"].toString();
        entry.date = obj["date"].toString();
        entry.text = obj["text"].toString();
        m_entries.append(entry);
        m_entryList->addItem(QString("[%1] %2").arg(entry.date, entry.title));
    }
}

void DiaryWindow::saveEntries() {
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dirPath);
    QString filePath = dirPath + QString("/diary_%1.json").arg(m_userId);

    QJsonArray array;
    for (const auto& entry : m_entries) {
        QJsonObject obj;
        obj["title"] = entry.title;
        obj["date"] = entry.date;
        obj["text"] = entry.text;
        array.append(obj);
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(array).toJson());
        file.close();
    }
}

