#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QMessageBox>

#include "task_client.h"
#include "result.h"
#include "blenderrenderengine.h"
#include "udpmanager.h"


class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    ~Client();
    QObject *myParent;
    bool is_running;

    //For receiving Tasks - those should be private/protected - testing purposes!
    QTcpServer *TCPServer;
    QTcpSocket *TCPSocket;

    //For sending the result back to the server:
    QTcpSocket *resultSendSocket;
    int totalResultBytes;
    int PayloadSize;
    int bytesResultToWrite;
    int bytesResultWritten;
    QByteArray customResultDataType;
    QDataStream out; //needed?

    UdpManager *UDPMan;

    QByteArray datagramBroadcast;

    QThread *workerThread;

    QByteArray readBytes;

    BlenderRenderEngine *RenderEngine;

    bool connected;
    bool working;
    bool gpuRendering;

    QHostAddress serverAddress;

    void start();
    void stop();

    void sendBroadcast(QString &message);

private:
    QString debugMessage;

signals:
    void processTaskClient(Task_Client &clientTask);

    //GUI:
    void updateClientConsole(const QString &message);
    void updateClientConnectedStyle(QString &style);
    void updateClientWorkingStyle(QString &style);

    void updateClientTCPProgressBar(quint8 &value);
    void updateClientTCPProgressBarByAmount(quint8 &amount);

    void resetClientProgressBars();



public slots:

    void onTCPServerNewConnection();
    void onTCPUpdateServerProgress();
    void onTCPConnectionClosed();
    void displayError(QAbstractSocket::SocketError);
    void onProcessTaskClient(Task_Client &clientTask);
    void onSetServerAddress(QHostAddress sender);

    //sending back the result:
    void onSendResultBack(Result result);

};

#endif // CLIENT_H
