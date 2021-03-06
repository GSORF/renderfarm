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
    QString framesRendered;
    QString framesTotal;

private:
    QThreadPool* threadPool;
    void incomingConnection(qintptr handle);
    QString formatTime(int msecs);


signals:

public slots:
    void onSetJobName(QString name);
    void onSetCurrentRenderThumbnail(QString path);
    void onSetCurrentRenderTime(int time);
    void onSetRemainingRenderTime(int time);
    void onSetFramesRendered(QString renderedFrames);
    void onSetFramesTotal(QString totalFrames);

};

#endif // WEBSERVER_H
