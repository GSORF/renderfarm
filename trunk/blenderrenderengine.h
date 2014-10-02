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



#ifndef BLENDERRENDERENGINE_H
#define BLENDERRENDERENGINE_H

#include <QObject>

#include <QFile>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QPixmap>
#include <QThread>
#include <QTextStream>
#include <QMessageBox>
#include <QDateTime>

#include "task_client.h"
#include "result.h"

class BlenderRenderEngine : public QObject
{
Q_OBJECT

public:
    BlenderRenderEngine(QObject *parent = 0);
    ~BlenderRenderEngine();

    void open(Task_Client &_theTask);
    void saveTaskFile();
    void renderTaskFile();
    void createResult();
    void loadRenderedImage();
    void setBlenderPath(QString path);
    QString getBlenderPath();
    void quit();

private:
    Task_Client BlenderTask;
    bool taskSaved;
    bool taskCached;

    //Create a blenderprocess:
    QProcess *blender;

    QString taskFilename;
    QString taskDirectory;
    QString renderFilename;
    QString renderFilenameLeft; //"rendering.L.0000.jpg"
    QString renderFilenameRight; //"rendering.R.0000.jpg"

    QString blenderPath;

    bool blenderError;
    bool blenderWarning;

    bool render2dSaved;
    bool render3dLeftSaved;
    bool render3dRightSaved;

    void displayLogMessage(const QString message, bool error);

signals:
    void updateClientGUIThumbnail(QPixmap image);
    void updateClientGUIConsole(const QString &message);
    void updateClientRenderStatus(quint8 percent);
    void sendResultBack(Result result);

public slots:
    void onEngineStarted();
    void onEngineBytesWritten(qint64);
    void onEngineError(QProcess::ProcessError);
    void onEngineFinished();
    void onEngineAboutToClose();
    void onEngineReadChannelFinished();
    void onEngineReadyRead();
    void onEngineDestroyed();

    void onSetFileCached(bool cached);
};

#endif // BLENDERRENDERENGINE_H
