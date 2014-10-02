#ifndef WEBSOCKETRUNNABLE_H
#define WEBSOCKETRUNNABLE_H

#include <QRunnable>
#include <QTcpSocket>
#include <QDebug>
#include <QTime>
#include <QStringList>
#include <QFile>
#include <QDir>

class WebSocketRunnable : public QRunnable
{
public:
    WebSocketRunnable(int handle);

    QString jobName;
    QString currentRenderThumbnail;
    QString currentRenderTime;
    QString remainingRenderTime;
    QString framesRendered;
    QString framesTotal;

    void run();
private:
    int descriptor;
};

#endif // WEBSOCKETRUNNABLE_H
