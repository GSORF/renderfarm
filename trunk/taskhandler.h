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



#ifndef TASKHANDLER_H
#define TASKHANDLER_H

#include <QtCore>


class TaskHandler
{

protected:
    struct taskStc {
        int stcStart;
        int stcTotalFrames;
        QString stcRender;
        bool stcCPU;
        bool stcStereo3D;
        QString stcFileName;
        QString jobId;
    };

    int numberOfClients;
    QQueue<taskStc> lowPiorityListCPU;
    QQueue<taskStc> lowPiorityListGPU;
    QQueue<taskStc> highPiorityListCPU;
    QQueue<taskStc> highPiorityListGPU;
    short lastUsedCpuQueue;
    short lastUsedGpuQueue;
    short scheduleMode;
    int jobCount;
public:
    TaskHandler();
    void addTask(int jobCount, short scheduleMode, int stcStart, int stcTotalFrames, QString stcRender, bool stcCPU, bool stcStereo3D, QString stcFileName, QString jobId);
    taskStc popCpuQueue();
    taskStc popGpuQueue();
    taskStc getCpuFirst();
    taskStc getGpuFirst();

    bool cpuQueueIsEmpty();
    bool gpuQueueIsEmpty();
    void setJobCount(int change);
    int getJobCount();




};

#endif // TASKHANDLER_H
