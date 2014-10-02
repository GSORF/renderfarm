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

