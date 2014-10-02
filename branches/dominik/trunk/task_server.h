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
    void initTask(QString filename, int startframe, int endframe, QString RenderEngine, bool CPU, bool stereo3D, QByteArray &file, QString jobId, bool cached);
    void sendTo(QString host, unsigned int port);
    void selfTo(QString host);

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
    int PayloadSize;
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

