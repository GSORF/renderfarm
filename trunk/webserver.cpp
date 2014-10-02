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



#include "webserver.h"

WebServer::WebServer(QObject *parent) :
    QTcpServer(parent)
{
    jobName = "- none -";
    currentRenderThumbnail = "";
    currentRenderTime = "--:--:--";
    remainingRenderTime = "--:--:--";
    remainingRenderTimeGSORF = "--:--:--";
    renderNodes = 1;
    framesRendered = "-";
    framesTotal = "-";

    threadPool = new QThreadPool(this);
    threadPool->setMaxThreadCount(10); //this is the maximum number of simultaneous connections
}

void WebServer::incomingConnection(qintptr handle)
{
    WebSocketRunnable *runnable = new WebSocketRunnable(handle);
    runnable->currentRenderThumbnail = currentRenderThumbnail;
    runnable->currentRenderTime = currentRenderTime;
    runnable->framesRendered = framesRendered;
    runnable->framesTotal = framesTotal;
    runnable->jobName = jobName;
    runnable->remainingRenderTime = remainingRenderTime;
    runnable->remainingRenderTimeGSORF = remainingRenderTimeGSORF;
    runnable->renderNodes = QString::number(renderNodes);

    threadPool->start(runnable);
}

QString WebServer::formatTime(qint64 msecs)
{
    QString formattedTime;

    qint64 hours = msecs/(1000*60*60);
    qint64 minutes = (msecs-(hours*1000*60*60))/(1000*60);
    qint64 seconds = (msecs-(minutes*1000*60)-(hours*1000*60*60))/1000;
    qint64 milliseconds = msecs-(seconds*1000)-(minutes*1000*60)-(hours*1000*60*60);

    formattedTime.append(QString("%1").arg(hours, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(minutes, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(seconds, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(milliseconds, 3, 10, QLatin1Char('0')));

    return formattedTime;
}

void WebServer::onSetJobName(QString name)
{
    this->jobName = name;
}

void WebServer::onSetCurrentRenderThumbnail(QString path)
{
    this->currentRenderThumbnail = path;
}

void WebServer::onSetCurrentRenderTime(qint64 time)
{
    this->currentRenderTime = formatTime(time);
}

void WebServer::onSetRemainingRenderTime(qint64 time)
{
    this->remainingRenderTime = formatTime(time);
    this->remainingRenderTimeGSORF = formatTime(time / ( (renderNodes < 1)?1:renderNodes ) );
}

void WebServer::onSetRenderNodes(qint32 numRenderNodes)
{
    this->renderNodes = numRenderNodes;
}

void WebServer::onSetFramesRendered(QString renderedFrames)
{
    this->framesRendered = renderedFrames;
}

void WebServer::onSetFramesTotal(QString totalFrames)
{
    this->framesTotal = totalFrames;
}

void WebServer::start()
{
    if(listen(QHostAddress::Any, 8080))
    {
        qDebug() << "Listening on Port 8080...";
    }
    else
    {
        qDebug() << "Error while starting the webserver on Port 8080 - is it already in use?";
    }
}

void WebServer::stop()
{

    if(isListening())
    {
        close();
        qDebug() << "Closed webserver: Not listening anymore!";
    }
}
