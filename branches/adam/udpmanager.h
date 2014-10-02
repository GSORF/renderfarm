#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QtNetwork>
#include <QHostAddress>
#include <openssl/md5.h>
#include <QCryptographicHash>

class UdpManager : public QObject
{
    Q_OBJECT
public:
    explicit UdpManager(QObject *parent = 0);
    void setStatusBusy();
    void setStatusAvailable();
    void setFileHash(QString hash);
    void setCPUWorker();
    void setGPUWorker();
    void start();
    void stop();
    bool is_running;

private:
    void writeUDP(QString status, QHostAddress receiver);
    bool workerIsBusy;
    QString file_hash;
    QUdpSocket *socket;
    bool isGPUWorker;

signals:
    void updateClientGUIConsole(const QString &message);
    void setServerAddress(QHostAddress sender);
    void setFileCached(bool cached);
private slots:
    void readUDP();
};

#endif // UDPMANAGER_H
