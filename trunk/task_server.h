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



#ifndef TASK_SERVER_H
#define TASK_SERVER_H

#include <QMessageBox>

#include "dialog.h"

class Task_Server : public QObject
{
    Q_OBJECT
public:
    Task_Server();
    ~Task_Server();
    void initTask(QString filename, int startframe, int endframe, QString RenderEngine, bool CPU, bool stereo3D, QByteArray &file, QString jobId, bool cached, int payloadSize);
    void sendTo(QString host, unsigned int port);
    void selfTo(QString host);
    int PayloadSize;

private:
    qint16 currentFrame;      //short
    qint16 totalFrames;        //short
    QByteArray file;
    QString RenderEngine;   //= "BLENDER", "CINEMA4D", "MAYA", "MAX", etc.
    bool CPU;               //GPU CPU Rendering
    bool stereo3D;          //stereoscopic 3D rendering
    bool taskCached;
    QString feedback;
    QString taskId;
    QString jobId;
    QString packetType;
    QString filename;
    QString filePath;

    QTcpSocket tcpClient;
    int TotalBytes;

    int bytesToWrite;
    int bytesWritten;
    QByteArray customDataType;
    QDataStream out;


signals:
    void sendProgress(int prozent);
    void sendJobSent (QString connParams);
    void processTaskClient(Task_Client &task);

public slots:
    void startTransfer();
    void updateClientProgress(qint64 numBytes);
    void displayError(QAbstractSocket::SocketError socketError);
};


#endif // TASK_SERVER_H

