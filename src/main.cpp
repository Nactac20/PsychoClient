#include <QApplication>
#include <QMessageBox>
#include <QFont>
#include "network/NetworkManager.h"
#include "auth/AuthWindow.h"
#include "client/ClientWindow.h"
#include "psychologist/PsychologistWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setFont(QFont("Segoe UI", 10));
    
    app.setStyleSheet(R"(
        QMainWindow, QDialog, QWidget {
            background-color: #FFFFFF;
            color: #1A365D;
        }
        
        QWidget#central {
            background-color: #F7FAFC;
        }
        
        QLabel {
            color: #2D3748;
            background: transparent;
        }
        
        QPushButton {
            background-color: #3182CE;
            color: white;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            border: none;
        }
        
        QPushButton:hover {
            background-color: #2B6CB0;
        }
        
        QPushButton:pressed {
            background-color: #2C5282;
        }
        
        QPushButton:disabled {
            background-color: #E2E8F0;
            color: #A0AEC0;
        }
        
        QLineEdit, QTextEdit, QComboBox, QDateTimeEdit, QSpinBox, QListWidget, QTableWidget {
            background-color: white;
            color: #1A365D;
            border: 1px solid #CBD5E0;
            border-radius: 6px;
            padding: 6px;
            selection-background-color: #BEE3F8;
            selection-color: #2C5282;
        }
        
        QLineEdit:focus, QTextEdit:focus, QComboBox:focus {
            border: 1px solid #3182CE;
        }
        
        QTableWidget {
            gridline-color: #E2E8F0;
            alternate-background-color: #F7FAFC;
        }
        
        QHeaderView::section {
            background-color: #EDF2F7;
            color: #4A5568;
            padding: 8px;
            border: none;
            border-bottom: 2px solid #E2E8F0;
            font-weight: bold;
        }
        
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #EDF2F7;
        }
        
        QListWidget::item:selected {
            background-color: #EBF8FF;
            color: #2C5282;
        }
        
        QGroupBox {
            font-weight: bold;
            color: #2C5282;
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            margin-top: 20px;
            padding-top: 15px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 10px;
        }

        QScrollBar:vertical {
            border: none;
            background: #F7FAFC;
            width: 10px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #CBD5E0;
            min-height: 20px;
            border-radius: 5px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            border: none;
            background: none;
        }
    )");
    
    NetworkManager::instance().connectToServer("localhost", 12345);
    
    AuthWindow authWindow;
    
    QObject::connect(&authWindow, &AuthWindow::loginSuccess, 
        [&authWindow](const QJsonObject& data) {
            authWindow.hide();
            
            int userId = data["user_id"].toInt();
            QString role = data["role"].toString();
            QString name = data["name"].toString();
            QString email = data["email"].toString();
            
            if (role == "client") {
                ClientWindow* clientWindow = new ClientWindow(userId, name, email);
                clientWindow->show();
            } else {
                PsychologistWindow* psychWindow = new PsychologistWindow(userId, name, email);
                psychWindow->setProfileData(
                    email,
                    data["specialization"].toString(),
                    data["education"].toString(),
                    data["description"].toString()
                );
                psychWindow->show();
            }
        });
    
    authWindow.show();
    
    return app.exec();
}
