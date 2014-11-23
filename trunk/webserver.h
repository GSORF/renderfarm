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



#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QTcpServer>
#include <QThreadPool>

#include "websocketrunnable.h"

class WebServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = 0);

    void start();
    void stop();

    QString jobName;
    QString currentRenderThumbnail;
    QString currentRenderTime;
    QString remainingRenderTime;
    QString remainingRenderTimeGSORF;
    qint32 renderNodes;
    QString framesRendered;
    QString framesTotal;

private:
    QThreadPool* threadPool;
    void incomingConnection(qintptr handle);
    QString formatTime(qint64 msecs);


signals:

public slots:
    void onSetJobName(QString name);
    void onSetCurrentRenderThumbnail(QString path);
    void onSetCurrentRenderTime(qint64 time);
    void onSetRemainingRenderTime(qint64 time);
    void onSetRenderNodes(qint32 numRenderNodes);
    void onSetFramesRendered(QString renderedFrames);
    void onSetFramesTotal(QString totalFrames);

};

#endif // WEBSERVER_H
