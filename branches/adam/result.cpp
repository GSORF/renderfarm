#include "result.h"

Result::Result()
{
    jobId = "";
    taskId = "";
    is_stereo3D = false;
}

Result::~Result()
{

}

Result::Result(QString _jobId, QString _taskId, qint16 _frame, bool _is_stereo3D, QImage &_render2D)
{
    jobId = _jobId;
    taskId = _taskId;
    frame = _frame;
    render2D = _render2D;
    is_stereo3D = _is_stereo3D;

}

Result::Result(QString _jobId, QString _taskId, qint16 _frame, bool _is_stereo3D, QImage &_render2D, QImage &_render3D_left, QImage &_render3D_right)
{
    jobId = _jobId;
    taskId = _taskId;
    frame = _frame;
    render2D = _render2D;
    render3D_left = _render3D_left;
    render3D_right = _render3D_right;
    is_stereo3D = _is_stereo3D;
}

//Serialization:
QDataStream &operator<<(QDataStream &out, const Result &result)
{
    out << result.jobId << result.taskId << result.frame << result.is_stereo3D << result.render2D << result.render3D_left << result.render3D_right;
    return out;
}

//De-Serialization:
QDataStream &operator>>(QDataStream &in, Result &result)
{
    QString jobId;
    QString taskId;
    qint16 frame;
    QImage render2D;
    QImage render3D_left;
    QImage render3D_right;
    bool is_stereo3D; //stereoscopic rendering

    in >> jobId >> taskId >> frame >> is_stereo3D >> render2D >> render3D_left >> render3D_right;
    result = Result(jobId, taskId, frame, is_stereo3D, render2D, render3D_left, render3D_right);
    return in;
}
