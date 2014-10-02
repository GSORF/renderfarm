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
    QString remainingRenderTimeGSORF;
    QString renderNodes;
    QString framesRendered;
    QString framesTotal;

    void run();
private:
    int descriptor;
};

#endif // WEBSOCKETRUNNABLE_H
