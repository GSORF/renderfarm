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



#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QtNetwork>
#include <QHostAddress>
#include <openssl/md5.h>
#include <QCryptographicHash>

class UdpManager : public QObject
{
    Q_OBJECT
public:
    explicit UdpManager(QObject *parent = 0);
    void setStatusBusy();
    void setStatusAvailable();
    void setFileHash(QString hash);
    void setCPUWorker();
    void setGPUWorker();
    void start();
    void stop();
    bool is_running;

private:
    void writeUDP(QString status, QHostAddress receiver);
    bool workerIsBusy;
    QString file_hash;
    QUdpSocket *socket;
    bool isGPUWorker;

signals:
    void updateClientGUIConsole(const QString &message);
    void setServerAddress(QHostAddress sender);
    void setFileCached(bool cached);
private slots:
    void readUDP();
};

#endif // UDPMANAGER_H
