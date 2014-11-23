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



#ifndef DIALOG_H
#define DIALOG_H


#include <QtCore>
#include <QTcpSocket>
#include <QtNetwork>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <QThread>
#include <QFileDialog>
#include <QListWidgetItem>
#if (QT_VERSION < 0x050000)
#include <QWebView>
#else 
#include <QtWebKitWidgets/QWebView>
#endif
#include <QUrl>
#include "taskhandler.h"
#include "client.h"
#include "task_server.h"
#include "webserver.h"

namespace Ui {
class Dialog;
}

namespace RenderNode{
    enum Icon
    {
        CPU=1,
        GPU, //2
        CPU_CACHED=4,
        GPU_CACHED //5
    };
    class Node
    {
    public:
        QString IP;
        bool cached;
        bool busy;
        bool online;
        Icon icon;
        qint64 timeStart; //Time since first message (added to queue)
        qint64 timeLast; //Time since most recent message (UDP-Update)

        QString toString(){ return IP; }
        void setCached(bool _cached){ cached = _cached; }
        void setBusy(bool _busy){ busy = _busy; }
        void setOnline(bool _online){ online = _online; }
    };

}

QDebug operator<<(QDebug dbg, const RenderNode::Node &node);

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();


protected :

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    TaskHandler myTaskHandler;
    WebServer myWebServer;

    int time_AutoSelectServer;
    qint64 applicationTimestamp;
    QString applicationTitle;
    void startGSORFServer();
    void stopGSORFServer();
    void initBroadcast();
    QMap<uint, QHostAddress> priorityMap; // key- random number, value- ip address
    QMap<QString, QString> internalTaskMap;
    QMap<QString, int> jobMap;
    void determineServer();
    struct taskStc {
        int stcStart;
        int stcTotalFrames;
        QString stcRender;
        bool stcCPU;
        bool stcStereo3D;
        QString stcFileName;
       // int tasksLeft;
        //int currentFrame;
        QString jobId;
    };
        //taskId +"#"+filePath+"#"+currentFrame+"#"+projektframes +"#"+CPU+"#"+stereo3D+"#"+cleintIP

   // QQueue<taskStc> listStcCPU; moved to TaskHandler
   // QQueue<taskStc> listStcGPU;
    QQueue<RenderNode::Node> listClientIpCPU;
    QQueue<RenderNode::Node> listClientIpGPU;
    QQueue<RenderNode::Node> listClientIPBusy;
    QQueue<int>taskRenderTime;
    int taskRenderTimeQueueSize;
    int framesSentCounter;
    qint32 numRenderNodes;

    uint longestWaitingTime;

    QStringList freeWorkers;
    QStringList freeGPUWorkers;

    QString file_url;
    QString file_name;
    QString file_hash;
    QTimer* myQtTimer;
    QTimer* myPriorityTimer;
    QTimer* myAutoSelectServerTimer;
    QTimer* myJobHubTimer;
    void refreshWorkers();
    void callAvailableWorkers();
    void callIntelligentWorkers();
    QString getWorkerAddress();
    QString getGPUWorkerAddress();
    void generateRandomGSORFServerPriority();
    uint serverPriority;
    void callAvailableServers();

    QTcpServer *tcpReceiveRendered;
    QTcpSocket *tcpRenderedResultSocket;
    QByteArray readBytes;

    void checkResultID(Result &result);
    void saveResultOnHarddisk(Result &result);
    void checkResultComplete(Result &result);
    void packToVideo(Result &result);
    void serverMsgLogger(QString msg);
    void resendTask(QString taskKey);

private:
    Ui::Dialog *ui;
    QUdpSocket udpSocket_client;
    QUdpSocket udpSocketRecieve_client;
    QUdpSocket udpSocketRecieve_priority;
    uint udpSendPort;
    uint udpRecievePort;
    uint tcpSendPort;
    uint tcpRecievePort;
    uint milliSecInMin;
    int randInt(int fromNum, int toNum);
    void taskRenderTimeManager(int taskRenderT);
    bool isServer;
    bool isSearchingForServer;
    bool int2BoolWorkaround(int myInt);
    short getJobPrio();
    void callAvailableWorkersPediodicly();
    bool file_cached;
    quint64 ipSumCPU;
    quint64 ipSumGPU;
    short ipCounter;
    quint64 calculateSumOfIPAdresses(QQueue<RenderNode::Node> renderNodeList);
    QListWidgetItem* createRenderNodeEntry(RenderNode::Node renderNode);
    void updateIPLists(QQueue<RenderNode::Node> &renderNodeList, QString senderIP, bool file_cached);

    QProcess *ffmpeg;
    //-----------------------------------------------------Client
    Client *myClient;
    //-----------------------------------------------------Client end

    ulong timVar;
    int PayloadSize;
    ulong timerSec;
    int timerMin;
    void initIdleTimer();
    void updateIdleTime();
    void dropEvent(QDropEvent *event);
signals:
    void udpConnectionCPUImpulse(QString cpuDetails);
    void udpConnectionGPUImpulse(QString gpuDetails);
    void sendIP2CPUWorkers(QString cpuListMember);
    void sendIP2GPUWorkers(QString gpuListMember);
    void taskSentSIGNAL(QStringList loggerStrList);
    //void taskRenderTimeSIGNAL(int taskTime);

    void setJobName(QString name);
    void setCurrentRenderThumbnail(QString path);
    void setCurrentRenderTime(qint64 time);
    void setRemainingRenderTime(qint64 time);
    void setFramesRendered(QString renderedFrames);
    void setFramesTotal(QString totalFrames);
    void setRenderNodes(qint32 numRenderNodes);
    //-----------------------------------------------------Client
    //-----------------------------------------------------Client end


public slots:
    void processPendingDatagrams();
    void cpuTaskDetails(QString cpuDetails);
    void gpuTaskDetails(QString gpuDetails);
    void getConnectionFeedback(QString feedbackCn);
    void stopPriorityBroadcastSlot();
    void processPriorityDatagrams();
    void tcpRenderedResultSlot();
    void onTCPUpdateServerProgress();
    void onTCPConnectionClosed();


private slots:
    void serverLoggerSLOT (QStringList loggerStrList);
    void onBtnClickedNuke();
    void onSetOptions_btn();

    void render_btn_clicked();
    void spinboxStart(int startFrameChange);
    void spinboxEnd(int endFrameChange);
    void refreshAutoServerDisplay();
    void setServer();
    void setClient();
    void initBroadcastSlot();
    void onBtnExpertMode();
    void onBtnSimpleMode();

    void onDebugBtnClicked ();
    void theJobHub();

    //-----------------------------------------------Client
    void onBtnPathToBlenderClicked();
    void onBtnStartStopClientToggled(bool checked);
    void onUpdateClientConnectedStyle(QString &style);
    void onUpdateClientWorkingStyle(QString &style);
    void onUpdateClientTCPProgressBar(quint8 &value);
    void onUpdateClientTCPProgressBarByAmount(quint8 &amount);
    void onResetClientProgressBars();

    void onRbCpuToggled(bool value);
    void onRbGpuToggled(bool value);
    void onRbInstalledClicked(bool value);
    void onRbPortableClicked(bool value);
    void onUpdateClientGUIThumbnail(QPixmap image);
    void onUpdateClientGUIConsole(const QString &message);
    void onUpdateClientRenderStatus(quint8 percent);
    //-----------------------------------------------Client end


};

#endif // DIALOG_H
