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



#include "dialog.h"
#include "ui_dialog.h"

QDebug operator<<(QDebug dbg, const RenderNode::Node &node)
{
     dbg << node.IP << "(cached=" << node.cached << ", icon=" << node.icon << ")";
     return dbg;
}

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    applicationTitle = this->windowTitle();

    //--------------------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------UI-SERVER-START------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------
    //                                                     #rrggbb
    this->setStyleSheet("color: #b1b1b1; background-color: #323239;");
    QImage cpuIcon;
    QImage gpuIcon;
    ui->btn_simplemode->hide();

    cpuIcon.load("assets/cpu.jpg");
    gpuIcon.load("assets/gpu.jpg");
    if((cpuIcon.isNull())||(gpuIcon.isNull())){
        ui->txtConsole->appendPlainText("ressources missing, please check the /assets/ folder");
    }
    ui->lbl_cpu->setPixmap(QPixmap::fromImage(cpuIcon));
    ui->lbl_gpu->setPixmap(QPixmap::fromImage(gpuIcon));
    //Hide Options
    ui->RenderOptionFrame->hide();
    ui->txtConsole->hide();

    if (cpuIcon.isNull())
        ui->lbl_cpu->setText("no image");
    if (gpuIcon.isNull())
        ui->lbl_gpu->setText("no image");
    ui->pb_sendFile->setValue(0);
    ui->txtConsole->appendPlainText("Please drop your .blend file here!");
    //--------------------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------UI-SERVER-END--------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------SERVER INITIAL LOGIC-START----------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------

    initIdleTimer();

    udpSendPort = 6668;
    udpRecievePort=6669;
    tcpSendPort=6667;
    tcpRecievePort=6682;
    milliSecInMin=60000;
    timVar=2500;
    numRenderNodes = 0;

    generateRandomGSORFServerPriority();

    udpSocketRecieve_client.bind(udpRecievePort);
    udpSocketRecieve_priority.bind(6670);

    tcpReceiveRendered = new QTcpServer(this);
    tcpReceiveRendered->listen(QHostAddress::Any, tcpRecievePort);
    tcpRenderedResultSocket = new QTcpSocket(this);

    connect( ui->btn_render, SIGNAL( clicked() ), this, SLOT(render_btn_clicked()),Qt::QueuedConnection );
    connect( ui->btn_nuke, SIGNAL( clicked() ), this, SLOT(onBtnClickedNuke()),Qt::QueuedConnection );
    connect( ui->btn_expertmode, SIGNAL(clicked() ), this, SLOT(onBtnExpertMode()),Qt::QueuedConnection);
    connect( ui->btn_simplemode, SIGNAL(clicked() ), this, SLOT(onBtnSimpleMode()),Qt::QueuedConnection);

    connect( ui->sb_start, SIGNAL(valueChanged(int)), this, SLOT(spinboxStart(int)),Qt::QueuedConnection);
    connect( ui->sb_end, SIGNAL(valueChanged(int)), this, SLOT(spinboxEnd(int)),Qt::QueuedConnection);
    myQtTimer = new QTimer(this);
    //connect( myQtTimer,SIGNAL(timeout()),this, SLOT(callAvailableWorkersPediodicly()),Qt::QueuedConnection); //associate timer to function
    //myQtTimer->setInterval(6050);
    myJobHubTimer = new QTimer(this);
    connect( myJobHubTimer,SIGNAL(timeout()),this, SLOT(theJobHub()),Qt::QueuedConnection);
    myJobHubTimer->setInterval(timVar);
    connect(this, SIGNAL(udpConnectionCPUImpulse(QString)),this, SLOT(cpuTaskDetails(QString)),Qt::QueuedConnection);
    connect(this, SIGNAL(udpConnectionGPUImpulse(QString)),this, SLOT(gpuTaskDetails(QString)),Qt::QueuedConnection);
    connect(ui->rb_Server, SIGNAL(toggled(bool)),this,SLOT(setServer()));
    connect(ui->rb_Client, SIGNAL(toggled(bool)),this,SLOT(setClient()));

    connect(ui->rb_AutoSelect, SIGNAL(toggled(bool)), this, SLOT(initBroadcastSlot()));

    connect(&udpSocketRecieve_priority, SIGNAL(readyRead()),this, SLOT( processPriorityDatagrams()),Qt::QueuedConnection);

    connect(tcpReceiveRendered, SIGNAL(newConnection()), this, SLOT(tcpRenderedResultSlot()));
    connect (ui->btn_Debug, SIGNAL(clicked()),this,SLOT(onDebugBtnClicked()),Qt::QueuedConnection);
    connect (this,SIGNAL(taskSentSIGNAL(QStringList)), this, SLOT(serverLoggerSLOT(QStringList)), Qt::QueuedConnection);

    connect(this, SIGNAL(setCurrentRenderThumbnail(QString)), &myWebServer, SLOT(onSetCurrentRenderThumbnail(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(setRemainingRenderTime(qint64)), &myWebServer, SLOT(onSetRemainingRenderTime(qint64)), Qt::QueuedConnection);
    connect(this, SIGNAL(setCurrentRenderTime(qint64)), &myWebServer, SLOT(onSetCurrentRenderTime(qint64)), Qt::QueuedConnection);
    connect(this, SIGNAL(setFramesRendered(QString)), &myWebServer, SLOT(onSetFramesRendered(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(setFramesTotal(QString)), &myWebServer, SLOT(onSetFramesTotal(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(setJobName(QString)), &myWebServer, SLOT(onSetJobName(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(setRenderNodes(qint32)), &myWebServer, SLOT(onSetRenderNodes(qint32)), Qt::QueuedConnection);

    //--------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------SERVER INITIAL LOGIC-END------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------CLIENT-START---------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------
    myClient = new Client(this);

    connect(ui->rbCpuRender, SIGNAL(toggled(bool)), this, SLOT(onRbCpuToggled(bool)));
    connect(ui->rbGpuRender, SIGNAL(toggled(bool)), this, SLOT(onRbGpuToggled(bool)));

    connect(ui->rbInstalled, SIGNAL(toggled(bool)), this, SLOT(onRbInstalledClicked(bool)));
    connect(ui->rbPortable, SIGNAL(toggled(bool)), this, SLOT(onRbPortableClicked(bool)));

    connect(ui->btnPath, SIGNAL(clicked()), this, SLOT(onBtnPathToBlenderClicked()));
    connect(ui->btnStartStopClient, SIGNAL(toggled(bool)), this, SLOT( onBtnStartStopClientToggled(bool)) );

    connect(myClient->RenderEngine, SIGNAL(updateClientGUIThumbnail(QPixmap)), this, SLOT(onUpdateClientGUIThumbnail(QPixmap)));
    connect(myClient->RenderEngine, SIGNAL(updateClientGUIConsole(const QString&)), this, SLOT(onUpdateClientGUIConsole(const QString&)));
    connect(myClient->RenderEngine, SIGNAL(updateClientRenderStatus(quint8)), this, SLOT(onUpdateClientRenderStatus(quint8)));

    connect(myClient->UDPMan, SIGNAL(updateClientGUIConsole(const QString&)), this, SLOT(onUpdateClientGUIConsole(const QString&)) );

    connect(myClient, SIGNAL(updateClientConsole(const QString&)), this, SLOT( onUpdateClientGUIConsole(const QString&)) );
    connect(myClient, SIGNAL(updateClientConnectedStyle(QString&)), this, SLOT(onUpdateClientConnectedStyle(QString&)) );
    connect(myClient, SIGNAL(updateClientWorkingStyle(QString&)), this, SLOT( onUpdateClientWorkingStyle(QString&)) );

    //--------------------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------CLIENT-END-----------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------
    myClient->stop();
    stopGSORFServer();
    isSearchingForServer = false;
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(1);
    time_AutoSelectServer = 7000;
    myPriorityTimer = new QTimer(this); //create timer

    file_hash = "Th15I5Th3Dummy5HasHOfGSORF.ORG";
    ipSumCPU = 0;
    ipSumGPU = 0;
    ipCounter = 5;

    framesSentCounter = 0;

    myAutoSelectServerTimer = new QTimer(this);
    myAutoSelectServerTimer->setInterval(200);
    myAutoSelectServerTimer->start();
    connect(myAutoSelectServerTimer, SIGNAL(timeout()), this, SLOT( refreshAutoServerDisplay()) );
    initBroadcast();
}


Dialog::~Dialog()
{
    QString byeByeMsg="Server dumped: \n";
    while(!jobMap.isEmpty()){
        byeByeMsg+=jobMap.begin().key()+"; \n";
        jobMap.erase(jobMap.begin());
    }
    serverMsgLogger(byeByeMsg);

    delete ui;
    delete myClient;
}
//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------CLIENT-START---------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------


void Dialog::onRbCpuToggled(bool value)
{
    if (value == true)
    {
        myClient->UDPMan->setCPUWorker();
    }
    else
    {
        myClient->UDPMan->setGPUWorker();
    }
}



void Dialog::onRbGpuToggled(bool value)
{
    if (value == true)
    {
        myClient->UDPMan->setGPUWorker();
    }
    else
    {
        myClient->UDPMan->setCPUWorker();
    }
}

void Dialog::onRbInstalledClicked(bool value)
{
    if(value)
    {
        myClient->RenderEngine->setBlenderPath("blender");
    }
}

void Dialog::onRbPortableClicked(bool value)
{
    if(value)
    {
        QString path = ui->txtBlenderPath->text();
        myClient->RenderEngine->setBlenderPath(path);
    }
}

void Dialog::onBtnPathToBlenderClicked()
{
    QString path = QFileDialog::getOpenFileName(this,tr("Choose Application"), QDir::currentPath(), tr("Applications (*.app *.exe *.run);; All files (*.*)"));
    ui->txtBlenderPath->setText(path);
    ui->rbPortable->setChecked(true);
    myClient->RenderEngine->setBlenderPath(path);
}
void Dialog::onBtnStartStopClientToggled(bool checked)
{
    if(myClient->is_running && checked)
    {
        ui->btnStartStopClient->setText("start Client");
        myClient->stop();
    }
    else
    {
        ui->btnStartStopClient->setText("stop Client");
        myClient->start();
    }
}

void Dialog::onUpdateClientConnectedStyle(QString &style)
{
    ui->lblConnected->setStyleSheet(style);
}

void Dialog::onUpdateClientWorkingStyle(QString &style)
{
    ui->lblWorking->setStyleSheet(style);
}

void Dialog::onUpdateClientTCPProgressBar(quint8 &value)
{
    ui->tcpProgressBar->setValue( value );
}

void Dialog::onUpdateClientTCPProgressBarByAmount(quint8 &amount)
{
    quint8 progress = ui->tcpProgressBar->value();
    if( progress < 100 )
    {
        progress+=amount;
    }
    ui->tcpProgressBar->setValue( progress );
}

void Dialog::onResetClientProgressBars()
{
    ui->tcpProgressBar->reset();
    ui->blenderProgressBar->reset();
}

void Dialog::onUpdateClientGUIThumbnail(QPixmap image)
{
    ui->txtConsoleClient->append("Pixmap updated! colorcount=" + QString::number(image.colorCount() ) + " depth:" +  QString::number( image.depth() ) + "height: " + QString::number( image.height()) + " width:" + QString::number( image.width() ) );

    int w = ui->lblThumbnailImage->width();
    int h = ui->lblThumbnailImage->height();

    // set a scaled pixmap to a w x h window keeping its aspect ratio
    ui->lblThumbnailImage->setPixmap(image.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));

}

void Dialog::onUpdateClientGUIConsole(const QString &message)
{
    ui->txtConsoleClient->append( QString::number(QDateTime::currentDateTime().time().hour()) + ":" + QString::number(QDateTime::currentDateTime().time().minute()) + ":" + QString::number(QDateTime::currentDateTime().time().second() ) + "|" + message);
}

void Dialog::onUpdateClientRenderStatus(quint8 percent)
{
    if(percent < 100)
    {
        this->setCursor(Qt::WaitCursor);
        ui->blenderProgressBar->setValue( percent );
    }
    if(percent == 200)
    {
        this->setCursor(Qt::ArrowCursor);
        ui->blenderProgressBar->reset();
        ui->lblWorking->setStyleSheet("color: white; background-color: red;");
        myClient->UDPMan->setStatusAvailable();
        return;
    }
}

//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------CLIENT-END-----------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------SERVER-START---------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------


void Dialog::dragEnterEvent(QDragEnterEvent *event){
    if(ui->rb_Server->isChecked())
        event->accept();
}

void Dialog::dragMoveEvent(QDragMoveEvent *event){
    if(ui->rb_Server->isChecked())
        event->accept();
}

void Dialog::dragLeaveEvent(QDragLeaveEvent *event){
    if(ui->rb_Server->isChecked())
        event->accept();
}

void Dialog::startGSORFServer()
{
    longestWaitingTime=0;
    taskRenderTimeQueueSize = 10; // 2 do ---size of the sliding window of task latencies

    udpSocketRecieve_client.bind(udpRecievePort);
    connect(&udpSocketRecieve_client, SIGNAL(readyRead()),this, SLOT(processPendingDatagrams()),Qt::QueuedConnection);
    connect(tcpReceiveRendered, SIGNAL(newConnection()), this, SLOT(tcpRenderedResultSlot()));
    refreshWorkers();
    //myQtTimer->start(); //check for clients
    myJobHubTimer->start(); //check for outstanding jobs and start sending them

    QDateTime now = QDateTime::currentDateTime();
    QString str;
    str = now.toString("dd/MM/yyyy hh:mm:ss");
    str = "Server running since: "+  str;
    serverMsgLogger(str);
    serverMsgLogger("JobId: projectFileAddress: renderMethod: Stereo3D: ClientIP: ");

    isServer=true;
    myWebServer.start();

    //QNetworkProxyFactory::setUseSystemConfiguration(true);
    QUrl url;
    url.setUrl("http://localhost");
    url.setPort(8080);
    ui->webView->load(url);
}

void Dialog::stopGSORFServer()
{
    disconnect(this, SLOT(processPendingDatagrams()));
    disconnect(this,  SLOT(tcpRenderedResultSlot()) );
    //udpSocketRecieve_client.close();
    udpSocketRecieve_client.disconnectFromHost();
    //myQtTimer->stop();
    myJobHubTimer->stop();

    isServer=false;
    myWebServer.stop();
}

void Dialog::initBroadcast()
{
    if(isSearchingForServer) return;
    isSearchingForServer = true;

    //Kill Server:
    //stopGSORFServer();

    //Kill Client:
    //myClient->stop();

    //Remove "Server" or "Client" Tab
    ui->tabWidget->removeTab(1);

    //Broadcast the randomServerPriority
    QByteArray DatagrammUDP;
    DatagrammUDP.append( "p#" + QString::number( serverPriority ) );
    udpSocket_client.writeDatagram(DatagrammUDP,QHostAddress::Broadcast, 6670);
    udpSocket_client.flush();
    //udpSocket_client.close();
    //disconnect(this,SLOT(processPriorityDatagrams()));
    qDebug()<<"Dialog::initBroadcast";

    applicationTimestamp = QDateTime::currentMSecsSinceEpoch() + time_AutoSelectServer;
    myPriorityTimer->singleShot(time_AutoSelectServer, this, SLOT(stopPriorityBroadcastSlot()));
}

void Dialog::determineServer()
{

    //qDebug()<<"smallest entry"<<priorityMap.lowerBound(0).key()<<priorityMap.lowerBound(0).value();

    //ask for available server
    callAvailableServers();

}

void Dialog::callAvailableWorkersPediodicly()
{
    if(listClientIpCPU.isEmpty() || listClientIpGPU.isEmpty())
        refreshWorkers();
}

quint64 Dialog::calculateSumOfIPAdresses(QQueue<RenderNode::Node> renderNodeList)
{
    quint64 sumofIPAdresses = 0;
    for( int i=0; i<renderNodeList.count(); i++ )
    {
        sumofIPAdresses += QString( renderNodeList.at(i).IP ).remove('.').toULongLong();
    }
    //ui->txtConsole->appendPlainText( "Dialog::calculateSumOfIPAdresses() " + QString::number(sumofIPAdresses) );
    return sumofIPAdresses;

}

QListWidgetItem *Dialog::createRenderNodeEntry(RenderNode::Node renderNode)
{
    QString _entry = "";
    _entry += renderNode.IP;
    if(renderNode.cached)
        _entry += " (cached, ";
    else
        _entry += " (not cached, ";
    if(renderNode.online)
        _entry += "online)";
    else
        _entry += "offline)";

    QListWidgetItem *newEntry = new QListWidgetItem();
    if(renderNode.icon == RenderNode::CPU)
    {
        newEntry->setIcon( QIcon("assets/cpu_icon.png") );
    }
    else if(renderNode.icon == RenderNode::GPU)
    {
        newEntry->setIcon( QIcon("assets/gpu_icon.png") );
    }
    newEntry->setText(_entry);
    if(renderNode.online)
    {
        newEntry->setTextColor( Qt::darkGreen );
    }
    else
    {
        newEntry->setTextColor( Qt::red );
    }
    return newEntry;

}

void Dialog::updateIPLists(QQueue<RenderNode::Node> &renderNodeList, QString senderIP, bool file_cached)
{
    qint64 lastUpdate = 0;
    for( int i=0; i<renderNodeList.count(); i++ )
    {

        if(renderNodeList.at(i).IP == senderIP && file_cached)
        {
            renderNodeList[i].setCached(true);
        }

        lastUpdate = (QDateTime::currentMSecsSinceEpoch() - renderNodeList.at(i).timeLast);
        if(lastUpdate <= 3* 2000)
        {
            renderNodeList[i].setOnline(true);
        }
        else
        {
            renderNodeList[i].setOnline(false);
        }

        qDebug()<< i << ". (" << renderNodeList.at(i).IP << renderNodeList.at(i).online << ") timeLast: " << renderNodeList.at(i).timeLast << " - timeStart: " << renderNodeList.at(i).timeStart << " - lastUpdate: " << lastUpdate;

    }

}

void Dialog::callAvailableWorkers()
{
    QByteArray DatagrammUDP ="BLENDER"; //broadcast question, if workers can do blender
    udpSocket_client.writeDatagram(DatagrammUDP,QHostAddress::Broadcast, udpSendPort);
    udpSocket_client.flush();
}

void Dialog::callIntelligentWorkers()
{
    //Send Hash via UDP
    QString message = "#" + this->file_hash;
    QByteArray DatagrammUDP = message.toUtf8(); //broadcast question, if workers can do blender
    udpSocket_client.writeDatagram(DatagrammUDP,QHostAddress::Broadcast, udpSendPort);
    udpSocket_client.flush();
}


void Dialog::spinboxStart(int startFrameChange)
{
    if (startFrameChange > ui->sb_end->value())
        ui->sb_start->setValue(ui->sb_end->value());
}
void Dialog::spinboxEnd(int endFrameChange)
{
    if (endFrameChange < ui->sb_start->value())
        ui->sb_end->setValue(ui->sb_start->value());
}

void Dialog::refreshWorkers()
{
    callAvailableWorkers();
    callIntelligentWorkers();
}

//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------SERVER-END-----------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------UI-START-------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------


void Dialog::refreshAutoServerDisplay()
{
    qint64 timeRemaining = applicationTimestamp - QDateTime::currentMSecsSinceEpoch();
    QString message = "Auto Select";
    if(timeRemaining > 0)
    {
        if(isSearchingForServer) message += " [searching...] ("; else message += " [waiting...] (";
        ui->rb_AutoSelect->setText(message + QString::number(timeRemaining/1000.0, 'f', 2) + " sec remaining)" );
    }
    else
    {
        ui->rb_AutoSelect->setText(message + " (done)");
        myAutoSelectServerTimer->stop();
    }
}



void Dialog::setServer()
{
    if(ui->rb_Server->isChecked()){


        startGSORFServer();
        ui->rb_Server->setText("Server (running)");
        this->setWindowTitle("[Server] " + applicationTitle);
        ui->rb_Server->setChecked(true);

        ui->tabWidget->removeTab(1);
        ui->tabWidget->insertTab(1, ui->ServerTab, "Server");

        myClient->stop();
        ui->rb_Client->setText("Client (stopped)");

        myAutoSelectServerTimer->stop();
        ui->rb_AutoSelect->setText("Auto Select (done)");
        ui->tabWidget->setCurrentIndex(1);
    }
}

void Dialog::setClient()
{
    if(ui->rb_Client->isChecked() || !isSearchingForServer){
        isSearchingForServer = true;
        stopGSORFServer();
        ui->rb_Server->setText("Server (stopped)");
        this->setWindowTitle("[Client] " + applicationTitle);

        ui->tabWidget->removeTab(1);
        ui->tabWidget->insertTab(1, ui->ClientTab, "Client");

        myClient->start();
        ui->rb_Client->setText("Client (running)");
        ui->rb_Client->setChecked(true);

        myAutoSelectServerTimer->stop();
        ui->rb_AutoSelect->setText("Auto Select (done)");
        ui->tabWidget->setCurrentIndex(1);
    }
}

void Dialog::initBroadcastSlot()
{
    //clicking on auto select
    if (ui->rb_AutoSelect->isChecked())
    {
        myAutoSelectServerTimer->start();

        myClient->stop();
        stopGSORFServer();
        ui->rb_Client->setText("Client (stopped)");
        ui->rb_Server->setText("Server (stopped)");
        this->setWindowTitle("[Auto Select] " + applicationTitle);
        if(isSearchingForServer && !myClient->is_running)
        {
            isSearchingForServer = false;
        }
        initBroadcast();
    }
}


void Dialog::onBtnExpertMode()
{
    ui->webView->hide();
    ui->RenderOptionFrame->show();
    ui->txtConsole->show();
    ui->btn_expertmode->hide();
    ui->btn_simplemode->show();
}


void Dialog::onBtnSimpleMode()
{
  ui->RenderOptionFrame->hide();
  ui->txtConsole->hide();
  ui->webView->show();
  ui->btn_simplemode->hide();
  ui->btn_expertmode->show();

}

void Dialog::onDebugBtnClicked()
{
    ui->txtConsole->appendPlainText("JobQueue: number of unfinished jobs " + QString::number(myTaskHandler.getJobCount()));

}


void Dialog::render_btn_clicked() // here we put the dropped file & file info in a struct-array
{
    QFile dropped_file (file_url);
    if (!dropped_file.open(QIODevice::ReadOnly)) //accessibility controll for file
    {
        ui->txtConsole->appendPlainText("no file selected");
    }else{
        int startFrame = ui->sb_start->value();
        int endFrame = ui->sb_end->value();
        int totalFrames = (endFrame - startFrame) + 1;
        QStringList tempStrProjName = file_url.split('/');
        QString tempJobStr = QString::number( QDateTime::currentMSecsSinceEpoch())+tempStrProjName.last();
        jobMap.insert(tempJobStr, totalFrames);

        short jobPrio = getJobPrio();

        for (int i=startFrame; i<=endFrame; i++) //iterrate dat frames
        {
            myTaskHandler.addTask(jobMap.size(), jobPrio, i, totalFrames, "BLENDER", ui->rb_CPU->isChecked(), ui->cb_stereo3D->isChecked(), file_url,  tempJobStr);
        }
        ui->txtConsole->appendPlainText("file in queue");
    }
    //----------------   cleaning up   ---------------------------------
    ui->sb_start->setValue(0);
    ui->sb_end->setValue(0);
    this->file_url.clear();
    dropped_file.close();
    ui->lbl_filename->setText("<insert file>");
}

void Dialog::onBtnClickedNuke()
{
    QByteArray DatagrammUDP ="killall";
    udpSocket_client.writeDatagram(DatagrammUDP,QHostAddress::Broadcast, udpSendPort);
    udpSocket_client.flush();
    ui->txtConsole->appendPlainText("Sending the term signal to all clients!");

}

void Dialog::dropEvent(QDropEvent *event){
    if(ui->rb_Server->isChecked()){
        const QMimeData *mimeData = event->mimeData();

        if (mimeData->hasUrls())
        {

            event->acceptProposedAction();
            //######################################## preparing and checking file #################################
            QList<QUrl> urlList = mimeData->urls();

            QUrl qurl = urlList.at(0);
            QString url = qurl.toString();

#ifdef WIN32
            if (url.startsWith("file:///")) //M$ Windoose, thank god Qt parses "/" to "\" at runtime
            {
                url = url.remove("file:///");
            }
#else
            if (url.startsWith("file://")) //unix needs the third "/" otherwize not a valid file address
            {
                url = url.remove("file://");
            }
#endif

            if (url.endsWith(".blend")){
                //   ui->txtConsole->appendPlainText("Oh my god! It's a BLENDER!");
                ui->txtConsole->appendPlainText(url);
                QString url_trimmed;
                if(url.size()>50)
                {
                    url_trimmed = "..." + url.right ( 50 );
                } else {
                    url_trimmed = url;
                }
                ui->lbl_filename->setText(url_trimmed);
                this->file_url = url;

                //Read file and get md5 hash
                QByteArray fileData;

                QFile file( url ); //e.g.: /.../build/awesome.blend
                if( file.open(QIODevice::ReadOnly) )
                {
                    fileData = file.readAll();
                    file.close();

                    //Generate MD5 Hash
                    QByteArray hashData = QCryptographicHash::hash(fileData, QCryptographicHash::Md5);

                    this->file_hash = hashData.toHex();
                    qDebug() << "Hex-Hash: " << this->file_hash;

                }
                else
                {
                    //Error
                    qDebug() << "Error reading file " << url << "!";

                }

            }else{
                ui->txtConsole->appendPlainText("Dude, what do you want from me? I don't know you!");
            }
        }
    }
}



//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------UI-END---------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------UDP/MD5-START--------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------

void Dialog::processPendingDatagrams()
{
    QByteArray datagram;
    QHostAddress sender;
    quint16 senderport;

    //receiving BLENDERxxx
    datagram.resize(udpSocketRecieve_client.pendingDatagramSize());

    udpSocketRecieve_client.readDatagram(datagram.data(),datagram.size(), &sender, &senderport);

    QString tempString = datagram.data();

    if(tempString.indexOf("BLENDER") != -1 )
    {
        //desiring 141.45.33.xxxBLENDER0
        tempString = sender.toString();
        tempString.append("%");
        tempString.append(datagram.data());

        //reading response from workers
        if (tempString.endsWith("1"))
        {
            QStringList listC = tempString.split("%");
            emit udpConnectionCPUImpulse(listC.first());
        }
        else if (tempString.endsWith("2"))
        {
            QStringList listG = tempString.split("%");
            emit udpConnectionGPUImpulse(listG.first());
        }

        //If something changed (a new computer joined/removed) - refresh IP Lists:
        if(calculateSumOfIPAdresses(listClientIpCPU) != ipSumCPU || calculateSumOfIPAdresses(listClientIpGPU) != ipSumGPU )
        {
            ipCounter--;
        }
    }
    else if(tempString.indexOf("#") == 0)
    {
        //Received a client file hash
        QString recordedHash = tempString.right(32);
        qDebug() << "Hash-Compare: client: " << recordedHash << " - server: " << file_hash;

        if( file_hash == recordedHash )
        {
            qDebug() << "Hashes match";
            file_cached = true;
        }
        else
        {
            qDebug() << "Hashes do not match";
            file_cached = false;
        }
        //Check if IPs are correctly set as cached and online:
        updateIPLists(listClientIpCPU, sender.toString(), file_cached);
        updateIPLists(listClientIpGPU, sender.toString(), file_cached);
    }

    if (tempString.endsWith("GSORFSERVER")){
        if (isServer){
            QByteArray DatagrammUDP ="s#"; //answer this is the server
            udpSocket_client.writeDatagram(DatagrammUDP,sender,6670);
            udpSocket_client.flush();
            //ui->txtConsole->appendPlainText( "This is the Server" );
        }
    }
}





void Dialog::cpuTaskDetails(QString cpuDetails)
{
    qDebug() << "new CPU Detail " << cpuDetails;
    qDebug() << "IP's (CPU): " << listClientIpCPU;

    for( int i=0; i<listClientIpCPU.count(); i++ )
    {
        if(listClientIpCPU.at(i).IP == cpuDetails)
        {
            listClientIpCPU[i].timeLast = QDateTime::currentMSecsSinceEpoch();
            return;
        }
    }
    qDebug() << "queuing CPU " << cpuDetails;

    RenderNode::Node newCPUNode;
    newCPUNode.IP = cpuDetails;
    newCPUNode.icon = RenderNode::CPU;
    newCPUNode.cached = false;
    newCPUNode.timeLast = newCPUNode.timeStart = QDateTime::currentMSecsSinceEpoch();
    listClientIpCPU.enqueue(newCPUNode);
}

void Dialog::gpuTaskDetails(QString gpuDetails)
{
    qDebug() << "new GPU Detail " << gpuDetails;
    qDebug() << "IP's (GPU): " << listClientIpGPU;

    for( int i=0; i<listClientIpGPU.count(); i++ )
    {
        if(listClientIpGPU.at(i).IP == gpuDetails)
        {
            listClientIpGPU[i].timeLast = QDateTime::currentMSecsSinceEpoch();
            return;
        }
    }
    qDebug() << "queuing GPU " << gpuDetails;

    RenderNode::Node newGPUNode;
    newGPUNode.IP = gpuDetails;
    newGPUNode.icon = RenderNode::GPU;
    newGPUNode.cached = false;
    newGPUNode.timeLast = newGPUNode.timeStart = QDateTime::currentMSecsSinceEpoch();
    listClientIpGPU.enqueue(newGPUNode);
}

void Dialog::theJobHub(){ //work in progress
    if (listClientIpCPU.isEmpty()&&listClientIpGPU.isEmpty()){
        updateIdleTime();
    }else{
        initIdleTimer();
    }

    //myJobHubTimer->stop(); --maybe we dont need to stop the timer. possible that queuedconnection does this for us. needs testing!!!
    while (!(listClientIpCPU.empty()) && !(myTaskHandler.cpuQueueIsEmpty())){
        QFile tempFile (myTaskHandler.getCpuFirst().stcFileName);
        tempFile.open(QIODevice::ReadOnly);
        QByteArray line = tempFile.readAll();
        tempFile.close();

        Task_Server taskInstance;              //create a one frame task

        taskInstance.initTask(myTaskHandler.getCpuFirst().stcFileName, myTaskHandler.getCpuFirst().stcStart, myTaskHandler.getCpuFirst().stcTotalFrames, myTaskHandler.getCpuFirst().stcRender, myTaskHandler.getCpuFirst().stcCPU, myTaskHandler.getCpuFirst().stcStereo3D,line, myTaskHandler.getCpuFirst().jobId, listClientIpCPU.first().cached);

        connect(&taskInstance, SIGNAL(sendProgress(int)),ui->pb_sendFile, SLOT(setValue(int)));
        connect(&taskInstance,SIGNAL(sendJobSent(QString)), this ,SLOT (getConnectionFeedback(QString)),Qt::QueuedConnection);
        connect(&taskInstance,SIGNAL(processTaskClient(Task_Client&)),myClient, SLOT(onProcessTaskClient(Task_Client&)));
        unsigned int port = 6667;
        if(listClientIpCPU.first().online)
        {
                    taskInstance.sendTo(listClientIpCPU.first().IP,port);
                    numRenderNodes++;
        }
        myTaskHandler.popCpuQueue();
        listClientIpCPU.dequeue();
    }
    while (!(listClientIpGPU.empty()) && !(myTaskHandler.gpuQueueIsEmpty())){
        QFile tempFile (myTaskHandler.getGpuFirst().stcFileName);
        tempFile.open(QIODevice::ReadOnly);
        QByteArray line = tempFile.readAll();
        tempFile.close();

        Task_Server taskInstance;              //create a one frame task
        taskInstance.initTask(myTaskHandler.getGpuFirst().stcFileName, myTaskHandler.getGpuFirst().stcStart, myTaskHandler.getGpuFirst().stcTotalFrames, myTaskHandler.getGpuFirst().stcRender, myTaskHandler.getGpuFirst().stcCPU, myTaskHandler.getGpuFirst().stcStereo3D,line, myTaskHandler.getGpuFirst().jobId, listClientIpGPU.first().cached);

        connect(&taskInstance, SIGNAL(sendProgress(int)),ui->pb_sendFile, SLOT(setValue(int)));
        connect(&taskInstance,SIGNAL(sendJobSent(QString)), this ,SLOT (getConnectionFeedback(QString)),Qt::QueuedConnection);
        connect(&taskInstance,SIGNAL(processTaskClient(Task_Client&)),myClient, SLOT(onProcessTaskClient(Task_Client&)));
        unsigned int port = 6667;
        if(listClientIpGPU.first().online)
        {
              taskInstance.sendTo(listClientIpGPU.first().IP,port);
              numRenderNodes++;
        }

        myTaskHandler.popGpuQueue();
        listClientIpGPU.dequeue();
    }
    emit setRenderNodes(numRenderNodes);



    //UPDATING IP Lists (separate function recommended):
    ui->listWidget_cpu->clear();
    ui->listWidget_gpu->clear();
    for(int i= 0; i<listClientIpCPU.count(); i++)
    {
    ui->listWidget_cpu->addItem( createRenderNodeEntry(listClientIpCPU.at(i)) );
    ui->txtConsole->appendPlainText( QString::number(i+1) + ". CPU Times: last=" + QString::number(listClientIpCPU.at(i).timeLast) + ", start=" + QString::number(listClientIpCPU.at(i).timeStart) );
    }
    for(int i= 0; i<listClientIpGPU.count(); i++)
    {
    ui->listWidget_gpu->addItem( createRenderNodeEntry(listClientIpGPU.at(i)) );
    ui->txtConsole->appendPlainText( QString::number(i+1) + ". GPU Times: last=" + QString::number(listClientIpCPU.at(i).timeLast) + ", start=" + QString::number(listClientIpCPU.at(i).timeStart) );
    }

    //DO NOT DELETE -> WILL BREAK CONNECTION
    if(ipCounter <= 0)
    {
        ipSumCPU = calculateSumOfIPAdresses(listClientIpCPU);
        ipSumGPU = calculateSumOfIPAdresses(listClientIpGPU);
        ipCounter = 5;
        listClientIpCPU.clear();
        listClientIpGPU.clear();
    }

    //need a check if a tas is taking too long an if it does, the function resendTask(internalTaskMap.key(QString)) is to be called
    if ((longestWaitingTime>0)  && (!internalTaskMap.begin().key().isEmpty()) ){ //removed && (taskRenderTime.size()> 0.5*taskRenderTimeQueueSize)
        QStringList tempGetId = internalTaskMap.begin().key().split('#');
        if((QDateTime::currentMSecsSinceEpoch() - tempGetId.first().toULong())> (longestWaitingTime * 2)){ //changed from 20 to 2 by Domi
            resendTask(internalTaskMap.begin().key());






            //ui->txtConsole->appendPlainText("Task successfully requeued: "+internalTaskMap.begin().key() + "old key");
        }
    }
    //myJobHubTimer->start(10000);
    callAvailableWorkersPediodicly();
}

//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------UDP/MD5-END----------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------



void Dialog::initIdleTimer()
{
    timerSec=0;
    timerMin=0;
    ui->lbl_idlemin->setText(QString::number(timerMin));
}

void Dialog::updateIdleTime()
{
    timerSec+=timVar;
    if(timerSec>=milliSecInMin){
        timerSec-=milliSecInMin;
        timerMin++;
        ui->lbl_idlemin->setText(QString::number(timerMin));
    }
}


void Dialog::taskRenderTimeManager(int taskRenderT)
{

    framesSentCounter++ ;

    ui->txtConsole->appendPlainText("resendCounter++ "+QString::number(framesSentCounter)+"!!");
    ui->txtConsole->appendPlainText("job render time:"+QString::number(taskRenderT)+" ms!");
    if (taskRenderTime.size()< taskRenderTimeQueueSize){
        taskRenderTime.enqueue(taskRenderT);
    } else {
        taskRenderTime.dequeue();
        taskRenderTime.enqueue(taskRenderT);
        //taskRenderTime[(randInt(0,taskRenderTime.size()))]=taskRenderT;
    }

    int wMax=0;
    for (int i=0; i<taskRenderTime.size(); i++) //get longest waiting time from 10 tasks
    {
        if (taskRenderTime.at(i) > wMax)
            wMax = taskRenderTime.at(i); //evtl summe???
    }

    this->longestWaitingTime=wMax;

}


void Dialog::getConnectionFeedback(QString feedbackCn)
{
    //taskId%otherParams
    //taskId +"%"+filePath+"@"+currentFrame+"@"+projekt frames +"@"+CPU+"@"+stereo3D+"@"+jobId+"@"+cleintIP

    QStringList tempStringlst = feedbackCn.split('%');
    internalTaskMap.insert(tempStringlst.first(), tempStringlst.last() );

    emit taskSentSIGNAL(tempStringlst);
}

void Dialog::stopPriorityBroadcastSlot()
{
    udpSocketRecieve_client.flush();
    disconnect(this, SLOT( processPriorityDatagrams()));

    if(priorityMap.size() > 0)
    {
        QString DebugMessage = "Smallest QMap-Entry: key: ";
        DebugMessage.append( QString::number( priorityMap.lowerBound(0).key() ) );
        DebugMessage.append( " - value: " );
        DebugMessage.append( priorityMap.lowerBound(0).value().toString() );
        //ui->txtConsole->appendPlainText( DebugMessage );

        determineServer();
    }
}

void Dialog::processPriorityDatagrams()
{
    //In UDP-Receive: Save the priorities in local array
    QByteArray datagram;
    QHostAddress sender;
    quint16 senderport;
    //expecting   p#NUMBER
    datagram.resize(udpSocketRecieve_priority.pendingDatagramSize());

    udpSocketRecieve_priority.readDatagram(datagram.data(),datagram.size(), &sender, &senderport);


    QString tempString;
    tempString = datagram.data();


    if( tempString.startsWith("p#") )
    {
        tempString = tempString.remove("p#");
        uint intTempString = tempString.toUInt();

        priorityMap.insert(intTempString, sender);
    }
    if(tempString.startsWith("s#")){
        if(!isServer)
        {
            setClient();
        }
    }
}



//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------PROGRESS-FEEDBACK-START----------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------



void Dialog::tcpRenderedResultSlot()
{
    qDebug() << "Dialog::tcpRenderedResultSlot()";
    if(tcpReceiveRendered->hasPendingConnections())
    {
        tcpRenderedResultSocket = tcpReceiveRendered->nextPendingConnection();

        readBytes.clear();

        qDebug() << "Dialog::tcpRenderedResultSlot() => tcpRenderedResultSocket->bytesAvailable() = " << tcpRenderedResultSocket->bytesAvailable();
        qDebug() << "Dialog::tcpRenderedResultSlot() => tcpRenderedResultSocket->bytesToWrite() = " << tcpRenderedResultSocket->bytesToWrite();

        if(tcpRenderedResultSocket->bytesAvailable()>0)
        {
            readBytes = tcpRenderedResultSocket->readAll();
            tcpRenderedResultSocket->waitForReadyRead();
        }

        qDebug() << "Dialog::tcpRenderedResultSlot() => sizeof(readBytes) = " << sizeof(readBytes);
        qDebug() << "Dialog::tcpRenderedResultSlot() => readBytes.size() = " << readBytes.size();

        connect(tcpRenderedResultSocket, SIGNAL(readyRead()), this, SLOT(onTCPUpdateServerProgress()));
        connect(tcpRenderedResultSocket, SIGNAL(disconnected()), this, SLOT(onTCPConnectionClosed()));
    }


}

void Dialog::onTCPUpdateServerProgress() //SIGNAL: ReadyRead
{
    readBytes.append( tcpRenderedResultSocket->readAll() );
}

void Dialog::onTCPConnectionClosed()
{
    QDataStream in(&readBytes, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_8);

    Result newResult;
    in >> newResult; //DeSerialize

    qDebug() << "Dialog::onTCPConnectionClosed() => sizeof(newResult) = " << sizeof(newResult);

    checkResultID(newResult);
}

void Dialog::serverLoggerSLOT(QStringList loggerStrList)
{
    QStringList loggerParamsList = loggerStrList.last().split('@');
    //JobId,projectFileAddress,renderMethod,Stereo3D, ClientIP
    QString templogStr;
    templogStr= "Task with taskId: "+loggerStrList.first()+" sent to: "  +loggerParamsList.last();
    serverMsgLogger(templogStr);
}


void Dialog::generateRandomGSORFServerPriority()
{
    //Make a random number
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    uint high = 30000;
    uint low = 0;

    serverPriority = qrand() % ((high + 1) - low) + low;
    //ui->txtConsole->appendPlainText( "serverPriority: " + QString::number(serverPriority) );

}

void Dialog::callAvailableServers()
{
    QByteArray DatagrammUDP ="GSORFSERVER"; //broadcast question, if workers can do blender
    udpSocket_client.writeDatagram(DatagrammUDP,QHostAddress::Broadcast  ,udpRecievePort);
    udpSocket_client.flush();
    qDebug() << "calling available Servers..";

    isSearchingForServer = false;
    myPriorityTimer->singleShot(5000, this, SLOT(initBroadcastSlot()) );
    applicationTimestamp = QDateTime::currentMSecsSinceEpoch() + 5000;


}

void Dialog::checkResultID(Result &result)
{
    if  (internalTaskMap.contains(result.taskId)){
        saveResultOnHarddisk(result);
        QString msg= "Saving file with TaskID: " + result.taskId;
        serverMsgLogger(msg);
    } else {
        QString msg= "Error! Incomming file with unknown TaskID: " + result.taskId;
        serverMsgLogger(msg);
        result.~Result();
    }
}

void Dialog::saveResultOnHarddisk(Result &result)
{
    //Create directory with JobID:
    QStringList tmpTaskId = result.taskId.split('#');
    QString resultDirectory = QApplication::applicationDirPath() + "/" + tmpTaskId.last() + "/";

    QDir dir(resultDirectory);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    if(!result.is_stereo3D)
    {
        //If 2D: Create Subdirectory "render2d"

        resultDirectory += "render2d/";
        dir = QDir(resultDirectory);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        //create "../render######.jpg"
        result.render2D.save(resultDirectory + "render" + tmpTaskId.at(1) + ".jpg", 0, 100);
    }
    else
    {
        qDebug() << "Saving 3D Rendering... ("<< result.render3D_left.width() << "x" << result.render3D_left.height() << ") and ("<< result.render3D_right.width() << "x" << result.render3D_right.height() << ")";

        //If 3D: Create Subdirectories "render2d" and "render3d"

        dir = QDir(resultDirectory + "render2d/");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        //create "../render######.jpg"
        result.render2D.save(resultDirectory + "render2d/render" + tmpTaskId.at(1) + ".jpg", 0, 100);

        dir = QDir(resultDirectory + "render3d/");
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        //Create Subdirectory "left"
        dir = QDir(resultDirectory + "render3d/left/");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        //create "../render######.jpg"
        result.render3D_left.save(resultDirectory + "render3d/left/render" + tmpTaskId.at(1) + ".jpg", 0, 100);

        //Create Subdirectory "right"
        dir = QDir(resultDirectory + "render3d/right/");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        //create "../render######.jpg"
        result.render3D_right.save(resultDirectory + "render3d/right/render" + tmpTaskId.at(1) + ".jpg", 0, 100);
    }

    jobMap[result.jobId]=jobMap.value(result.jobId)-1;

    internalTaskMap.remove(result.taskId);
    //calculate job latency

    qint64 lastTaskTime = QDateTime::currentMSecsSinceEpoch() - tmpTaskId.first().toULong();
    taskRenderTimeManager(lastTaskTime); //save task render time after file saved on hdd

    //Connection to the HTML-Monitor
    int framesTotal = tmpTaskId.at(2).toInt();
    int framesRendered = framesTotal - jobMap[result.jobId];
    qint64 currentRenderTime = lastTaskTime;
    qint64 remainingRenderTime = lastTaskTime * (framesTotal - framesRendered);

    result.render2D.scaledToWidth(300, Qt::SmoothTransformation).save(QApplication::applicationDirPath() + "/htdocs/images/thumb.jpg", 0, 75);

    emit setCurrentRenderThumbnail("images/thumb.jpg");
    emit setCurrentRenderTime(currentRenderTime);
    emit setRemainingRenderTime(remainingRenderTime);
    emit setJobName(result.jobId);
    emit setFramesRendered(QString::number(framesRendered));
    emit setFramesTotal(QString::number(framesTotal));

    numRenderNodes--;
    emit setRenderNodes(numRenderNodes);

    checkResultComplete(result);
}

void Dialog::checkResultComplete(Result &result)
{
    QStringList tempStringList = result.taskId.split('#');


         if (tempStringList.at(2).toInt() == framesSentCounter ){
            framesSentCounter = 0;

            // we are done.
            ui->txtConsole->appendPlainText("project "+result.jobId+ " finished successfuly" );
            jobMap.remove(result.jobId);
            myTaskHandler.setJobCount(-1);
            packToVideo(result);

    }

}

void Dialog::packToVideo(Result &result)
{

    QStringList tmpTaskId = result.taskId.split('#');
    QString resultDirectory = QApplication::applicationDirPath() + "/" + tmpTaskId.last() + "/";
    ui->txtConsole->appendPlainText("ffmpeg -r 25 -i " + resultDirectory + "render2d/render%d.jpg " + resultDirectory +"render2d/GSORF.avi");
    if(ui->cb_ffmpeg->isChecked())
    {


        //QString command = "ffmpeg -r 25 -i \"/home/grpc/Desktop/renderfarm/rf/branches/dominik/build-gsorf-Desktop-Debug/5 Blend_alpha.blend/render2d/render%d.jpg\" \"/home/grpc/Desktop/renderfarm/rf/branches/dominik/build-gsorf-Desktop-Debug/5 Blend_alpha.blend/render2d/render2d.mp4\"";

        QString command =  "ffmpeg -r 25 -i " + resultDirectory + "render2d/render%d.jpg " + resultDirectory +"render2d/render_2dVideo.mp4";
        ffmpeg->startDetached(command);
        ui->txtConsole->appendPlainText("project rendering Video");


    }
}

void Dialog::serverMsgLogger(QString msg)
{
    QFile fileServerLog("assets/out.txt");
    fileServerLog.open(QIODevice::ReadWrite | QIODevice::Append| QIODevice::Text);
    QTextStream outLogger(&fileServerLog);
    outLogger << msg  +"\n";
    fileServerLog.close();
}

void Dialog::resendTask(QString taskKey) //not yet tested
{
    //JobId: projectFileAddress: renderMethod: Stereo3D: ClientIP
    //key: 1400230411823#0#1#BMW1M-MikePa.blend value: /home/vergil/Downloads/BMW1M-MikePa.blend@1@0@jobid@192.168.1.4



    QStringList tempListKeySplit = taskKey.split('#');
    QStringList tempListValueSplit = internalTaskMap.value(taskKey).split('@');
    myTaskHandler.addTask(jobMap.size(), 0, tempListKeySplit.at(1).toInt(),  tempListKeySplit.at(2).toInt(), "BLENDER", int2BoolWorkaround(tempListValueSplit.at(1).toInt()), int2BoolWorkaround(tempListValueSplit.at(2).toInt()), tempListValueSplit.at(0), tempListValueSplit.at(3) );

    internalTaskMap.remove(taskKey);
    ui->txtConsole->appendPlainText("job successfully requeued! ");
}

int Dialog::randInt(int fromNum, int toNum)
{
    return qrand() % ((toNum + 1) - fromNum) + fromNum;
}

bool Dialog::int2BoolWorkaround(int myInt)
{
    if (myInt == 0){
        return false;
    }else{
        return true;
    }
}

short Dialog::getJobPrio()
{
    if (ui->rb_high->isChecked()){
        return 0;
    } else if (ui->rb_norml->isChecked()){
        return 1;
    } else {
        return 2;
    }

}

//--------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------PROGRESS-FEEDBACK-END------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
