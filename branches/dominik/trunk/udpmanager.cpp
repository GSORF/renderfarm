#include "udpmanager.h"

UdpManager::UdpManager(QObject *parent) :
    QObject(parent)

{
    is_running = true;

    workerIsBusy = false;
    isGPUWorker = false;
    file_hash = "Th15I5Th3DummymD5HasH0fGSORF.0RG";
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::Any, 6668);   //bind to non-specific address and certain port; our subnet is 141.75.33.255
    connect(socket,SIGNAL(readyRead()),this,SLOT(readUDP()));
}

void UdpManager::writeUDP(QString status, QHostAddress receiver)
{
    QByteArray DatagramUDP;
    DatagramUDP.clear();
    DatagramUDP.append(status);
    socket->writeDatagram(DatagramUDP,receiver,6669);
    QString debugMessage = "Sent \"" + QString(DatagramUDP.data()) + "\" to " + receiver.toString() + ":"+ QString::number(6669) + " (UDP)";
    emit updateClientGUIConsole(debugMessage);
}

void UdpManager::readUDP()
{
    QByteArray Temp;
    Temp.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(Temp.data(),Temp.size(),&sender,&senderPort);

    QString debugMessage = "Got \"" + QString(Temp.data()) + "\" from " + sender.toString() + ":"+ QString::number(senderPort) + " (UDP)";
    emit updateClientGUIConsole(debugMessage);

    QString compareString = "BLENDER";
    QString message;
    if(Temp.data() == compareString)
    {
        if (workerIsBusy)
            message = "BLENDER0";       //is busy
        else{
            if (isGPUWorker)
                message = "BLENDER2";   // is available GPU-Worker
            else
                message = "BLENDER1";   // is available CPU-Worker
        }
        writeUDP(message, sender);
    }
    else if( QString(Temp.data()).at(0) == '#' && !workerIsBusy )
    {
        message = "";

        QString filepathToBlend = QDir::currentPath() + "/awesome.blend";

        QByteArray fileData;

        QFile file( filepathToBlend ); //e.g.: /.../build/awesome.blend
        if( file.open(QIODevice::ReadOnly) )
        {
            fileData = file.readAll();
            file.close();

            //Generate MD5 Hash
            QByteArray hashData = QCryptographicHash::hash(fileData, QCryptographicHash::Md5);
            file_hash = hashData.toHex();

            qDebug() << "Hex-Hash: " << hashData.toHex();

        }
        else
        {
            //Error
            file_hash = "Th15I5Th3DummymD5HasH0fGSORF.0RG";
        }

        //Send UDP Response
        writeUDP( "#" + file_hash, sender );
        if(file_hash==QString(Temp.data()).remove('#') )
        {
            emit setFileCached(true);
        }
        else
        {
            emit setFileCached(false);
        }
    }
    else if( QString(Temp.data()).startsWith("killall"))
    {
    qDebug()<<"Bye Bye Blender";
    QProcess *myProcess = new QProcess(this);
    myProcess->start("killall blender");
    myProcess->waitForFinished();
    }


    emit setServerAddress(sender);

}

void UdpManager::setStatusAvailable()
{
    workerIsBusy = false;
}

void UdpManager::setFileHash(QString hash)
{
    file_hash = hash;
}

void UdpManager::setStatusBusy()
{
    workerIsBusy = true;
}

void UdpManager::setCPUWorker()
{
    isGPUWorker = false;
}

void UdpManager::setGPUWorker()
{
    isGPUWorker = true;
}

void UdpManager::start()
{
    if(is_running == false)
    {
        socket->bind(QHostAddress::Any, 6668);   //bind to non-specific address and certain port; our subnet is 141.75.33.255
        connect(socket,SIGNAL(readyRead()),this,SLOT(readUDP()));

        is_running = true;
    }
}

void UdpManager::stop()
{
    if(is_running == true)
    {
        socket->close();
        disconnect(socket,SIGNAL(readyRead()),this,SLOT(readUDP()));

        is_running = false;
    }
}
