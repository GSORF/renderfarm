#ifndef TASK_CLIENT_H
#define TASK_CLIENT_H

#include <QObject>
#include <QDataStream>

class Task_Client
{
public:
    Task_Client();
    Task_Client(QString _jobId, QString _taskId, qint16 _startframe, qint16 _endframe, QString &_renderengine, bool _cpu, bool _stereo3D, QByteArray &_file);

    QString jobId;
    QString taskId;
    qint16 startFrame; //short
    qint16 endFrame; //short
    QByteArray file;
    QString renderEngine; //= "BLENDER", "CINEMA4D", "MAYA", "MAX", etc.
    bool cpu; //GPU CPU Rendering
    bool stereo3D; //stereoscopic 3D rendering

signals:

public slots:

};

QDataStream &operator<<(QDataStream &out, const Task_Client &task);
QDataStream &operator>>(QDataStream &in, Task_Client &task);

#endif // TASK_CLIENT_H

