#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <functional>
#include <queue>

class NetworkManager : public QObject {
    Q_OBJECT
public:
    static NetworkManager& instance();
    
    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;
    
    void sendRequest(const QJsonObject& request,
                     std::function<void(const QJsonObject&)> callback);
    
signals:
    void connected();
    void disconnected();
    void error(const QString& message);
    
private:
    NetworkManager();
    ~NetworkManager();
    
    QThread* m_networkThread;
    QTcpSocket* m_socket = nullptr;
    QByteArray m_buffer;
    
    struct PendingRequest {
        QByteArray requestId;
        std::function<void(const QJsonObject&)> callback;
    };
    std::queue<PendingRequest> m_pendingRequests;
    
    void processData();
    static QByteArray generateRequestId();
    
    void initInThread();
};
