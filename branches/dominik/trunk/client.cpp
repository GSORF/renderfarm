#include "client.h"

Client::Client(QObject *parent) :
    QObject(parent)
{
    myParent = parent;
    is_running = true;

    //Listen on UDP (Broadcast):
    UDPMan = new UdpManager(this);

    //Listen on TCP:
    TCPServer = new QTcpServer(this);
    TCPSocket = new QTcpSocket(this);
    TCPServer->listen(QHostAddress::Any, 6667);
    connect(TCPServer, SIGNAL(newConnection()), this, SLOT(onTCPServerNewConnection() ));

    RenderEngine = new BlenderRenderEngine();

    workerThread = new QThread();
    RenderEngine->moveToThread(workerThread);
    workerThread->start();

    connect(this, SIGNAL(processTaskClient(Task_Client&)), this, SLOT(onProcessTaskClient(Task_Client&)));
    connect(RenderEngine, SIGNAL(sendResultBack(Result)), this, SLOT(onSendResultBack(Result)), Qt::QueuedConnection);
    connect(UDPMan, SIGNAL(setServerAddress(QHostAddress)), this, SLOT(onSetServerAddress(QHostAddress)));

    connect(UDPMan, SIGNAL(setFileCached(bool)), RenderEngine, SLOT(onSetFileCached(bool)));

}

Client::~Client()
{
    qDebug() << "destroying Client...";

    workerThread->terminate();
    workerThread->exit();
    workerThread->quit();

    UDPMan->deleteLater();

    TCPServer->deleteLater();
    TCPSocket->deleteLater();

    RenderEngine->deleteLater();
    workerThread->deleteLater();
}

void Client::start()
{
    if(is_running == false)
    {

        TCPServer->listen(QHostAddress::Any, 6667);
        UDPMan->start();
        connect(TCPServer, SIGNAL(newConnection()), this, SLOT(onTCPServerNewConnection() ));
        workerThread->start();

        is_running = true;

    }
}

void Client::stop()
{
    if(is_running == true)
    {
        TCPServer->close();
        TCPSocket->close();
        UDPMan->stop();
        disconnect(TCPServer, SIGNAL(newConnection()), this, SLOT(onTCPServerNewConnection() ));


        workerThread->exit();

        is_running = false;

    }
}




void Client::onTCPServerNewConnection()
{
    if(TCPServer->hasPendingConnections())
    {
        QString msg = "\nNew Connection incoming!";
        QString style = "color: black; background-color: green;";
        emit updateClientConsole( msg );
        emit updateClientConnectedStyle( style );
        connected = true;


        TCPSocket = TCPServer->nextPendingConnection();
        UDPMan->setStatusBusy();

        readBytes.clear();

        readBytes = TCPSocket->readAll();
        //TCPSocket->waitForReadyRead();

        connect(TCPSocket, SIGNAL(readyRead()), this, SLOT(onTCPUpdateServerProgress()));
        connect(TCPSocket, SIGNAL(disconnected()), this, SLOT(onTCPConnectionClosed()));
        connect(TCPSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    }

}


void Client::onTCPUpdateServerProgress() //SIGNAL: ReadyRead
{
    debugMessage = "Bytes available: " + QString::number( TCPSocket->bytesAvailable() );
    emit updateClientConsole( debugMessage );

    readBytes.append( TCPSocket->readAll() );

    quint8 progress = 10;
    emit updateClientTCPProgressBarByAmount(progress);


    debugMessage = "Now the size is: " + QString::number( readBytes.size() ) ;
    emit updateClientConsole( debugMessage );
}

void Client::onTCPConnectionClosed()
{
    QDataStream in(&readBytes, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_8);

    Task_Client newTask;
    in >> newTask; //DeSerialize

    debugMessage = "taskID: " + newTask.taskId + "\n";
    debugMessage.append("startFrame: " + QString::number(newTask.startFrame) + "\n" );
    debugMessage.append("totalFrames: " + QString::number(newTask.endFrame) + "\n");
    debugMessage.append("RenderEngine: " + newTask.renderEngine + "\n");

    if(newTask.cpu)
        debugMessage.append("CPU? Yes\n");
    else
        debugMessage.append("CPU? No\n");

    if(newTask.stereo3D)
        debugMessage.append("Stereo3D? Yes\n");
    else
        debugMessage.append("Stereo3D? No\n");

    debugMessage.append("Size of .blend-File: " + QString::number(newTask.file.size()) + " Bytes (~" +  QString::number(newTask.file.size() / 1024 / 1024) + " MB)\n");

    quint8 value = 0;
    emit updateClientTCPProgressBar(value);
    QString style = "color: white; background-color: red;";
    emit updateClientConnectedStyle(style);
    connected = false;

    emit processTaskClient(newTask);

    debugMessage.append("----------------------------------\n");
    debugMessage.append("BytesAvailable:" + QString::number( TCPSocket->bytesAvailable() ) + "\n");
    debugMessage.append("BytesToWrite:" + QString::number( TCPSocket->bytesToWrite() ) + "\n");
    debugMessage.append("localAdress:" +  TCPSocket->localAddress().toString() + "\n");
    debugMessage.append("localPort:" + QString::number( TCPSocket->localPort() ) + "\n");
    debugMessage.append("objectNames:" +  TCPSocket->objectName() + "\n");
    debugMessage.append("peerAddress:" + TCPSocket->peerAddress().toString() + "\n");
    debugMessage.append("peerName:" +  TCPSocket->peerName() + "\n");
    debugMessage.append("SocketSize:" + QString::number( TCPSocket->size() ) + "\n");

    emit updateClientConsole(debugMessage);
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    if (socketError == QTcpSocket::RemoteHostClosedError) //Verbindung wurde vom RemoteHost geschlossen, also raus hier
        return;

    //Show Network Error:
    QMessageBox::information(NULL, tr("Network error"), tr("The following error occurred: %1.").arg(TCPServer->errorString()));

    //Cleanup:
    TCPServer->close();
    TCPSocket->close();
    UDPMan->setStatusAvailable();

    emit resetClientProgressBars();
    debugMessage = "TCPSocket Error: " + TCPSocket->errorString();
    emit updateClientConsole( debugMessage );
}

void Client::onProcessTaskClient(Task_Client &clientTask)
{
    UDPMan->setStatusBusy();
    QString style = "color: black; background-color: green;";
    emit updateClientWorkingStyle(style);
    RenderEngine->open(clientTask);
}

void Client::onSendResultBack(Result result)
{
    UDPMan->setStatusBusy();

    qDebug() << "Client::onSendResultBack(Result result):begin => sizeof(result) = " << sizeof(result);

    qDebug() << "sending back result to server:" << ((result.is_stereo3D)?"It is a stereo result":"It is not a stereo result");
    emit resetClientProgressBars();

    QString style = "color: white; background-color: red;";
    emit updateClientConnectedStyle(style);

    qDebug() << "Client::onSendResultBack(): Sending back JobId:" << result.taskId << " to IP:" << serverAddress.toString();



    //send result back to Server via TCP
    resultSendSocket = new QTcpSocket(this);

    resultSendSocket->connectToHost(serverAddress, 6682);

    if(resultSendSocket->waitForConnected(5000)) {     //establish a tcp-connection on a GigabitLAN timeout

        quint8 progress;

        QByteArray customDataType;
        QDataStream out(&customDataType, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_8);

        //serialize
        out << result;

        //Sending:
        quint32 payload = 64 * 1024; //64kb
        quint32 bytesToWrite = (quint32)customDataType.size();

        qDebug() << "CLIENT: bytesToWrite="<< QString::number(bytesToWrite);
        if (bytesToWrite<payload)
        {
            //All at once:
            resultSendSocket->write(customDataType);
            resultSendSocket->flush();
            resultSendSocket->waitForBytesWritten(30000);

            progress = 100;
            emit updateClientTCPProgressBar(progress);
        } else {
            //By chunks:
            quint32 wieoft = bytesToWrite / payload; //TotalBytes / 64kb
            qint32 bytesToWrite2 = bytesToWrite;
            for(unsigned int i = 0; i <= wieoft; i++)
            {
                //0-63
                //64-127
                //128-192

                quint32 sovielBytes = qMin(payload,bytesToWrite);
                QByteArray ByteArrayPart = customDataType.mid(i*payload, sovielBytes );
                resultSendSocket->write(ByteArrayPart);
                resultSendSocket->flush();
                resultSendSocket->waitForBytesWritten(30000);

                progress += (quint8) (payload*100/(double)bytesToWrite2);
                emit updateClientTCPProgressBar(progress);

                bytesToWrite -= sovielBytes;


            }
            progress = 100;
            emit updateClientTCPProgressBar(progress);
        }

        resultSendSocket->close();                    // and we close


        emit resetClientProgressBars();

    } else {
        qDebug() << "CLIENT: resultSendSocket: NOT CONNECTED!";
        emit resetClientProgressBars();
    }


    UDPMan->setStatusAvailable();

}

void Client::onSetServerAddress(QHostAddress sender)
{
    serverAddress = sender;
}
