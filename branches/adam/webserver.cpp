#include "webserver.h"

WebServer::WebServer(QObject *parent) :
    QTcpServer(parent)
{
    jobName = "- none -";
    currentRenderThumbnail = "";
    currentRenderTime = "--:--:--";
    remainingRenderTime = "--:--:--";
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
