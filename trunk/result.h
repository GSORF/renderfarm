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
