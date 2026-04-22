#include "NetworkManager.h"
#include <QHostAddress>
#include <QRandomGenerator>

NetworkManager& NetworkManager::instance() {
    static NetworkManager instance;
    return instance;
}

NetworkManager::~NetworkManager() {
    disconnectFromServer();
}

void NetworkManager::connectToServer(const QString& host, quint16 port) {
    if (m_socket) {
        disconnectFromServer();
    }
    
    m_socket = new QTcpSocket(this);
    
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::disconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, [this](QAbstractSocket::SocketError) {
        emit error(m_socket->errorString());
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::processData);
    
    m_socket->connectToHost(host, port);
}

void NetworkManager::disconnectFromServer() {
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    while (!m_pendingRequests.empty()) {
        if (m_pendingRequests.front().callback) {
            QJsonObject error;
            error["status"] = "error";
            error["message"] = "Disconnected from server";
            m_pendingRequests.front().callback(error);
        }
        m_pendingRequests.pop();
    }
}

bool NetworkManager::isConnected() const {
    return m_socket && m_socket->state() == QTcpSocket::ConnectedState;
}

void NetworkManager::sendRequest(const QJsonObject& request,
                                  std::function<void(const QJsonObject&)> callback) {
    if (!isConnected()) {
        if (callback) {
            QJsonObject error;
            error["status"] = "error";
            error["message"] = "Not connected to server";
            callback(error);
        }
        return;
    }
    
    QJsonObject requestWithId = request;
    QByteArray requestId = generateRequestId();
    requestWithId["request_id"] = QString::fromUtf8(requestId);
    
    m_pendingRequests.push({requestId, callback});
    
    QJsonDocument doc(requestWithId);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    m_socket->write(data);
}

void NetworkManager::processData() {
    m_buffer.append(m_socket->readAll());
    
    while (m_buffer.contains('\n')) {
        int pos = m_buffer.indexOf('\n');
        QByteArray line = m_buffer.left(pos);
        m_buffer.remove(0, pos + 1);
        
        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isNull()) continue;
        
        QJsonObject response = doc.object();
        
        if (!m_pendingRequests.empty()) {
            auto& pending = m_pendingRequests.front();
            if (pending.callback) {
                pending.callback(response);
            }
            m_pendingRequests.pop();
        }
    }
}

QByteArray NetworkManager::generateRequestId() {
    quint64 id = QRandomGenerator::global()->generate64();
    return QByteArray::number(id);
}
