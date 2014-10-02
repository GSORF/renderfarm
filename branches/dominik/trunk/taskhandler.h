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
