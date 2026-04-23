#include "NetworkManager.h"
#include <QHostAddress>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QPointer>

NetworkManager& NetworkManager::instance() {
    static NetworkManager instance;
    return instance;
}

NetworkManager::NetworkManager() {
    m_networkThread = new QThread();
    m_networkThread->setObjectName("NetworkThread");
    this->moveToThread(m_networkThread);
    m_networkThread->start();
    
    QMetaObject::invokeMethod(this, &NetworkManager::initInThread, Qt::BlockingQueuedConnection);
}

NetworkManager::~NetworkManager() {
    if (m_networkThread) {
        m_networkThread->quit();
        m_networkThread->wait();
        delete m_networkThread;
    }
}

void NetworkManager::initInThread() {
    m_socket = new QTcpSocket();
    
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::disconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, [this](QAbstractSocket::SocketError) {
        emit error(m_socket->errorString());
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::processData);
}

void NetworkManager::connectToServer(const QString& host, quint16 port) {
    QMetaObject::invokeMethod(this, [this, host, port]() {
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->disconnectFromHost();
        }
        m_socket->connectToHost(host, port);
    }, Qt::QueuedConnection);
}

void NetworkManager::disconnectFromServer() {
    QMetaObject::invokeMethod(this, [this]() {
        if (m_socket) {
            m_socket->disconnectFromHost();
        }
        
        while (!m_pendingRequests.empty()) {
            auto req = m_pendingRequests.front();
            if (req.callback) {
                QJsonObject err;
                err["status"] = "error";
                err["message"] = "Disconnected from server";
                QMetaObject::invokeMethod(qApp, [cb = req.callback, err]() { cb(err); });
            }
            m_pendingRequests.pop();
        }
    }, Qt::QueuedConnection);
}

bool NetworkManager::isConnected() const {
    return m_socket && m_socket->state() == QTcpSocket::ConnectedState;
}

void NetworkManager::sendRequest(const QJsonObject& request,
                                 std::function<void(const QJsonObject&)> callback) {
    QByteArray id = generateRequestId();
    
    QMetaObject::invokeMethod(this, [this, request, callback, id]() {
        if (!isConnected()) {
            if (callback) {
                QJsonObject error;
                error["status"] = "error";
                error["message"] = "Not connected to server";
                QMetaObject::invokeMethod(qApp, [callback, error]() { callback(error); });
            }
            return;
        }
        
        m_pendingRequests.push({id, callback});
        
        QJsonObject fullRequest = request;
        fullRequest["request_id"] = QString(id);
        
        QByteArray data = QJsonDocument(fullRequest).toJson(QJsonDocument::Compact) + "\n";
        m_socket->write(data);
    }, Qt::QueuedConnection);
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
        QByteArray id = response["request_id"].toString().toUtf8();
        
        if (!m_pendingRequests.empty() && m_pendingRequests.front().requestId == id) {
            auto req = m_pendingRequests.front();
            m_pendingRequests.pop();
            
            if (req.callback) {
                QMetaObject::invokeMethod(qApp, [cb = req.callback, response]() {
                    cb(response);
                });
            }
        }
    }
}

QByteArray NetworkManager::generateRequestId() {
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    QByteArray id;
    for (int i = 0; i < 8; ++i) {
        id.append(alphabet[QRandomGenerator::global()->bounded(36)]);
    }
    return id;
}
