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



#include "task_client.h"


Task_Client::Task_Client()
{
    jobId="";
    taskId = "";
    startFrame = 0;
    endFrame = 0;


    renderEngine = "NONE";
    cpu = true;
    stereo3D = false;
}

Task_Client::Task_Client(QString _jobId, QString _taskId, qint16 _startframe, qint16 _endframe, QString &_renderengine, bool _cpu, bool _stereo3D, QByteArray &_file)
{
    jobId = _jobId;
    taskId = _taskId;
    startFrame = _startframe;
    endFrame = _endframe;
    renderEngine = _renderengine;
    cpu = _cpu;
    stereo3D = _stereo3D;
    file = _file;
}

//Serialization:
QDataStream &operator<<(QDataStream &out, const Task_Client &task)
{
    QString packetType = "GSORF_TASK";
    out << packetType << task.jobId << task.taskId << task.startFrame << task.endFrame << task.renderEngine << task.cpu << task.stereo3D << task.file;
    return out;
}

//De-Serialization:
QDataStream &operator>>(QDataStream &in, Task_Client &task)
{
    QString jobId;
    QString taskId;
    qint16 startFrame;
    qint16 endFrame;
    QByteArray file;
    QString renderEngine;
    bool cpu;
    bool stereo3D;

    QString packetType;

    in >> packetType;
    if(packetType == "GSORF_TASK")
    {
        //Handle a the dissecting of a Task-Datastream:
        in >> jobId >> taskId >> startFrame >> endFrame >> renderEngine >> cpu >> stereo3D >> file;
        task = Task_Client(jobId, taskId, startFrame, endFrame, renderEngine, cpu, stereo3D, file);
    }
    else
    {
        //Handle something else (e.g. a Result-Datastream):
        task = Task_Client();
    }

    return in;

}
