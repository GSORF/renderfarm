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



#include "websocketrunnable.h"

WebSocketRunnable::WebSocketRunnable(int handle) : descriptor(handle)
{
    jobName = "- none -";
    currentRenderThumbnail = "";
    currentRenderTime = "--:--:--";
    remainingRenderTime = "--:--:--";
    remainingRenderTimeGSORF = "--:--:--";
    renderNodes = "?";
    framesRendered = "-";
    framesTotal = "-";
}

void WebSocketRunnable::run()
{
    QTcpSocket *socket = new QTcpSocket();
    socket->setSocketDescriptor(descriptor);

    socket->waitForReadyRead();
    QString request = QString(socket->readAll());
    qDebug() << request;

    /*
     * EXAMPLE GET FOR IMAGE:
     *
    // GET /images/bg-gsorf.png HTTP/1.1
    // Host: localhost
    // User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0
    // Accept: image/png,image;q=0.8;q=0.5
    // Accept-Language: de,en-US;q=0.7,en;q=0.3
    // Accept-Encoding: gzip, deflate
    // Referer: http://localhost/
    // Connection: keep-alive
    */

    QString documentRoot = QDir::currentPath() + "/htdocs";

    QStringList splitRequestParts = request.split(' ');
    QString content;
    QByteArray response = "HTTP/1.1 200 OK\r\n";

    if(splitRequestParts.count() > 2)
    {
        QString filename = splitRequestParts.at(1);
        qDebug() << "SO THIS FILE SHOULD BE LOADED: " << filename;

        //Could be:
        // "/"
        // "/images/gsorf_feature_2.png"
        // "/images/icons-os-3.gif"
        // "/movie/gsorf_movie.mp4"

        if(filename == "/")
        {
            //Load the index.html
            qDebug() << "loading " + documentRoot + filename + "index.html...";
            QFile file(documentRoot + filename + "index.html");
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                qDebug()<<"...failed!";
                return;
            }
            else
            {
                content = QString( file.readAll() );


                content.replace("{JOB_NAME}", jobName);
                if(currentRenderThumbnail == "")
                {
                    content.replace("{CURRENT_RENDER_THUMBNAIL}", "images/missing.jpg");
                }
                else
                {
                    content.replace("{CURRENT_RENDER_THUMBNAIL}", currentRenderThumbnail);
                }

                content.replace("{CURRENT_RENDER_TIME}", currentRenderTime);
                content.replace("{REMAINING_RENDER_TIME}", remainingRenderTime);
                content.replace("{RENDER_NODES}", renderNodes);
                content.replace("{REMAINING_RENDER_TIME_GSORF}", remainingRenderTimeGSORF);
                content.replace("{FRAMES_RENDERED}", framesRendered);
                content.replace("{TOTAL_FRAMES}", framesTotal);
                content.replace("{TIME}", QTime::currentTime().toString());
                response += content.toLatin1();
                qDebug()<<"...success!";
            }
        }
        else
        {
            //Load the image or movie or something else
            QString filepath = documentRoot + filename;

            qDebug() << "loading " + filepath + "...";
            QFile file(filepath);
            if (!file.open(QIODevice::ReadOnly))
            {
                qDebug()<<"...failed!";
                return;
            }
            else
            {
                response += "Last-Modified: Tue, 22 Jul 2014 16:31:06 GMT\r\n";
                if(filename.endsWith(".jpg")) {
                    response += "Content-Type: image/jpeg\r\n";
                }else if(filename.endsWith(".png")) {
                    response += "Content-Type: image/png\r\n";
                }else if(filename.endsWith(".gif")) {
                    response += "Content-Type: image/gif\r\n";
                }else if(filename.endsWith(".mp4")) {
                    response += "Content-Type: video/mp4\r\n";
                }else if(filename.endsWith(".html") || filename.endsWith(".htm")) {
                    response += "Content-Type: text/html\r\n";
                }else if(filename.endsWith(".css")) {
                    response += "Content-Type: text/css\r\n";
                }
                response += "Content-Length: " + QString::number(file.size()) + "\r\n";
                response += "Server: GSORF/0.0.1 (2014-07-24)\r\n";
                response += "Date: Tue, 22 Jul 2014 16:31:06 GMT\r\n";
                response += "Connection: Keep-Alive\r\n\r\n";

                response += file.readAll();
                qDebug()<<"...success!";
            }

        }
    }

    //Write reponse:
    socket->write(response);
    socket->flush();
    socket->waitForBytesWritten();

    //Close connection and cleanup as soon as ressources aren't needed anymore
    socket->disconnectFromHost();
    socket->close();
    socket->deleteLater();
}
