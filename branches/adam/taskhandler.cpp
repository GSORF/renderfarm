#include "taskhandler.h"


TaskHandler::TaskHandler()
{
    this->jobCount=0;
    this->lastUsedCpuQueue=4;
    this->lastUsedGpuQueue=4;
}

void TaskHandler::addTask(int jobCount, short scheduleMode, int stcStart, int stcTotalFrames, QString stcRender, bool stcCPU, bool stcStereo3D, QString stcFileName, QString jobId)
{
    this->jobCount=jobCount;
    switch(scheduleMode){
    case 0  : //highest priority! this queue has precedent over the regular and low priority queue
    {
        struct taskStc currentTask = {stcStart, stcTotalFrames, stcRender, stcCPU, stcStereo3D, stcFileName, jobId};   //create a stuct instance with job information
        if (currentTask.stcCPU){
            highPiorityListCPU.enqueue(currentTask); //put job struct at end of cpu workers list
        }else{
            highPiorityListGPU.enqueue(currentTask);
        }
    }

        break;

    case 1  : //normal priority! mix tasks from different projekts in the queue

        // Advanced Scheduler-code

        break;

    case 2  : //lowest priority! FIFO - add task at the end of the queue
    {
        struct taskStc currentTask = {stcStart, stcTotalFrames, stcRender, stcCPU, stcStereo3D, stcFileName, jobId};   //create a stuct instance with job information
        if (currentTask.stcCPU){
            lowPiorityListCPU.enqueue(currentTask); //put job struct at end of cpu workers list
        }else{
            lowPiorityListGPU.enqueue(currentTask);
        }
    }
        break;

    default :
        qDebug()<<"TaskHandler Switch: we should never get here";

    }



}

TaskHandler::taskStc TaskHandler::popCpuQueue()
{
    if(lastUsedCpuQueue==0){
        lastUsedCpuQueue=4;
        return highPiorityListCPU.dequeue();
    }
    return lowPiorityListCPU.dequeue();
}

TaskHandler::taskStc TaskHandler::popGpuQueue()
{
    if(lastUsedGpuQueue==0){
        lastUsedGpuQueue=4;
        return highPiorityListGPU.dequeue();
    }
    return lowPiorityListGPU.dequeue();
}

TaskHandler::taskStc TaskHandler::getCpuFirst()
{
    if (!highPiorityListCPU.isEmpty()){
        lastUsedCpuQueue=0;
        return highPiorityListCPU.first();
    }
    return lowPiorityListCPU.first();
}

TaskHandler::taskStc TaskHandler::getGpuFirst()
{
    if (!highPiorityListGPU.isEmpty()){
        lastUsedGpuQueue=0;
        return highPiorityListGPU.first();
    }
    return lowPiorityListGPU.first();
}

bool TaskHandler::cpuQueueIsEmpty()
{
    return (lowPiorityListCPU.isEmpty()&&highPiorityListCPU.isEmpty());
}

bool TaskHandler::gpuQueueIsEmpty()
{
    return (lowPiorityListGPU.isEmpty()&&highPiorityListGPU.isEmpty());
}

void TaskHandler::setJobCount(int change)
{
    this->jobCount+=change;
}

int TaskHandler::getJobCount()
{
    return this->jobCount;
}
