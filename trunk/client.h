/*

Copyright 2013, 2014 Technische Hochschule Georg-Simon-Ohm, Dominik Rusiecki, Adam Kalisz

This file is part of GSORF.

GSORF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Foobar is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

*/



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
