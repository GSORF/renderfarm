#include "task_server.h"



Task_Server::Task_Server(){}

Task_Server::~Task_Server(){}

void Task_Server::initTask(QString filePath, int startframe, int endframe, QString RenderEngine, bool CPU, bool stereo3D, QByteArray &file, QString jobId, bool cached)
{
    this->filePath=filePath;
    QStringList fileAddress = filePath.split("/");
    this->filename = fileAddress.last();
    this->currentFrame = startframe;
    this->totalFrames = endframe;
    this->file = file;
    this->RenderEngine = RenderEngine;
    this->CPU = CPU;
    this->stereo3D = stereo3D;
    this->packetType = "GSORF_TASK";
    this->jobId = jobId;
    this->taskCached = cached;


    this->TotalBytes = 0;
    this->PayloadSize = 64; //64 * 1024; //64kb;
    this->bytesWritten = 0;
    this->bytesToWrite = 0;

    connect(&tcpClient, SIGNAL(connected()), this, SLOT(startTransfer()));
    connect(&tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));
    connect(&tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}


void Task_Server::sendTo(QString host, unsigned int port)
{

    // serialization thanx 2 ADAM:
    customDataType = QByteArray();
    QDataStream out(&customDataType, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);

    //jobId = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");  //Write Date like this: 05-Sep-12
    this->taskId = QString::number( QDateTime::currentMSecsSinceEpoch() ) + "#" + QString::number(this->currentFrame)+ "#" + QString::number(this->totalFrames) + "#" + this->filename; // get current timestamp + miliseconds as jobid

    //qDebug() << "TaskId=" << taskId;
    //stuff the drain
    out << this->packetType << this->jobId << this->taskId << this->currentFrame << this->totalFrames << this->RenderEngine << this->CPU << this->stereo3D;
    if(this->taskCached)
    {
        QByteArray emptyFile;
        qDebug() << "emptyFile has " << emptyFile.size() << "bytes";
        out<<emptyFile;
    }
    else
    {
        qDebug() << "'Thisfile' has " << this->file.size() << "bytes";
        out<<this->file;
    }

    bytesWritten = 0;
    bytesToWrite = 0;
    TotalBytes = customDataType.size();


    this->feedback = this->taskId +"%"+this->filePath +"@"+QString::number(this->CPU)+"@"+QString::number(this->stereo3D)+"@"+jobId+"@"+host;
    emit sendJobSent(this->feedback);

    tcpClient.connectToHost(host, port);

    tcpClient.waitForDisconnected(); //<-- if things go too quick, use this

    emit sendProgress(0);

}

void Task_Server::startTransfer()
{
    bytesToWrite = TotalBytes - (int)tcpClient.write( customDataType.mid(0, PayloadSize) ); //Writing first batch of Bytes
    qDebug() << "bytesToWrite="<< QString::number(bytesToWrite);
    //Possible bug: What if we are sending a file smaller than payload Size (64 kB) -> Doc says it just takes the whole QByteArray?
}

void Task_Server::updateClientProgress(qint64 numBytes)
{
    //Called when bytes are written:

    bytesWritten += (int)numBytes;

    if(bytesToWrite > 0 && tcpClient.bytesToWrite() <= 4* PayloadSize)
    {
        bytesToWrite -= (int)tcpClient.write( customDataType.mid( bytesWritten, qMin(bytesToWrite, PayloadSize) ) );
    }

    int progress = 0;
    progress = (int) ( (double) (TotalBytes / bytesWritten) * 100 );
    emit sendProgress(progress);

    qDebug() << "bytesWritten:\t" << bytesWritten;
    qDebug() << "TotalBytes:  \t" << TotalBytes;

    if(bytesWritten == TotalBytes)
    {
        tcpClient.close();
        emit sendProgress(100); // actualy we don't need it, just want to ensure there is no 99%-finish because of casting the variables
        emit sendJobSent(this->feedback);
    }
}

void Task_Server::displayError(QAbstractSocket::SocketError socketError)
{
    if(socketError == QTcpSocket::RemoteHostClosedError)
    {
        return;
    }

        tcpClient.close();

    this->feedback = "socket: NOT CONNECTED! " + tr("The following error occurred: %1.").arg(tcpClient.errorString());
    emit sendProgress(0);
    emit sendJobSent(this->feedback);
}


