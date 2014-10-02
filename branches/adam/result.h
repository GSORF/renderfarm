#ifndef RESULT_H
#define RESULT_H

#include <QObject>
#include <QImage>

class Result
{
public:
    Result();
    ~Result();

    //Constructor for 2D Image:
    Result(QString _jobId, QString _taskId, qint16 _frame, bool _is_stereo3D, QImage &_render2D);
    //Constructor for 3D Image:
    Result(QString _jobId, QString _taskId, qint16 _frame, bool _is_stereo3D, QImage &_render2D, QImage &_render3D_left, QImage &_render3D_right);

    QString jobId;
    QString taskId;
    qint16 frame;
    bool is_stereo3D; //stereoscopic rendering
    QImage render2D;
    QImage render3D_left;
    QImage render3D_right;

};

QDataStream &operator<<(QDataStream &out, const Result &result);
QDataStream &operator>>(QDataStream &in, Result &result);

#endif // RESULT_H
