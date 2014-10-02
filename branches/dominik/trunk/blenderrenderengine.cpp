#include "blenderrenderengine.h"

BlenderRenderEngine::BlenderRenderEngine(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<Result>("Result");

    taskSaved = false;
    taskCached = false;
    blender = new QProcess();

    blenderPath = "blender";
    blenderError = false;
    blenderWarning = false;

    connect(blender, SIGNAL(started()), this, SLOT(onEngineStarted()));
    connect(blender, SIGNAL(bytesWritten(qint64)), this, SLOT(onEngineBytesWritten(qint64)));
    connect(blender, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onEngineError(QProcess::ProcessError)));
    connect(blender, SIGNAL(finished(int)), this, SLOT(onEngineFinished()));
    connect(blender, SIGNAL(aboutToClose()), this, SLOT(onEngineAboutToClose()));
    connect(blender, SIGNAL(readChannelFinished()), this, SLOT(onEngineReadChannelFinished()));
    connect(blender, SIGNAL(readyRead()), this, SLOT(onEngineReadyRead()));
    connect(blender, SIGNAL(destroyed()), this, SLOT(onEngineDestroyed()));
    connect(blender, SIGNAL(readyReadStandardOutput()), this, SLOT(onEngineReadyRead()));
}

BlenderRenderEngine::~BlenderRenderEngine()
{
    //deletePointers; free memory!
    blender->deleteLater();
}

void BlenderRenderEngine::open(Task_Client &_theTask)
{
    emit updateClientRenderStatus(0);

    BlenderTask = _theTask;

    render2dSaved = false;
    render3dLeftSaved = false;
    render3dRightSaved = false;

    blenderError = false;
    blenderWarning = false;

    saveTaskFile();
}

void BlenderRenderEngine::saveTaskFile()
{
    //saving the .blend from the QByteArray onto the harddisk:
    emit updateClientRenderStatus(2);

    taskFilename = "awesome.blend";
    taskDirectory = QDir::currentPath();

    displayLogMessage( "TaskDirectory and TaskFilename: " + taskDirectory + ";" + taskFilename, false);

    if (!taskCached) {
        QFile file( taskDirectory + "/" + taskFilename ); //e.g.: /.../build/awesome.blend
        if( file.open(QIODevice::WriteOnly) && !(BlenderTask.file.size() <= 0) )
        {
            file.write(BlenderTask.file);
            file.close();
            taskSaved = true;
            emit updateClientRenderStatus(3);
        }
        else
        {
            //Error
            blenderError = true;
            displayLogMessage("Error creating file " + taskFilename + "!", true);
            quit();
            return;
        }
    }
    else
    {
        taskSaved = true;
    }

    renderTaskFile();
}

void BlenderRenderEngine::renderTaskFile()
{
  /* ////////////////////////
    //  The Blender Part  //
   //////////////////////// */
    emit updateClientRenderStatus(4);

    //Now we can open the .blend in Blender:

    renderFilename = "awesomeRender_######";
    QString command = blenderPath + " -b \""+ taskDirectory + "/" + taskFilename + "\" -o \"" + taskDirectory + "/" + renderFilename + "\" -F JPEG -x 1 -s " + QString::number(BlenderTask.startFrame) + " -e " + QString::number(BlenderTask.startFrame) + " -a";

    displayLogMessage("Command = " + command, false);

    // blender [-b <dir><file> [-o <dir><file>][-F <format>]
    // [-x [0|1]][-t <threads>][-S <name>][-f <frame>]
    // [-s <frame> -e <frame> -a]] [[-P <scriptname> [-- <parameter>]]

    //Animation:
    //blender -b awesome.blend -o file_##### -x 1 -s 2 -e 3 -a
    //Stillframe:
    //blender -b awesome.blend -o file_##### -x 1 -f 5

    renderFilename = "awesomeRender_" + QString("%1").arg( QString::number(BlenderTask.startFrame), 6, QChar('0')).toUpper() + ".jpg";
    displayLogMessage("RenderFilename: " + renderFilename, false);


    emit updateClientRenderStatus(5);


    blender->start(command);
    blender->waitForStarted();

}

void BlenderRenderEngine::createResult()
{

    Result finishedRendering;

    qDebug() << "BlenderRenderEngine::createResult():begin => Size of finishedRendering: " << sizeof(finishedRendering);

    finishedRendering.jobId = BlenderTask.jobId;
    finishedRendering.taskId = BlenderTask.taskId;

    if(BlenderTask.stereo3D)
    {
        //Add 2D Image:
        finishedRendering.render2D = QImage(renderFilename);

        //Add 3D Images:
        if(renderFilenameLeft != "" && renderFilenameRight != "")
        {
            qint64 startTime = QDateTime::currentMSecsSinceEpoch();

            QDir leftDir = QDir(renderFilenameLeft);
            leftDir.makeAbsolute();
            QDir rightDir = QDir(renderFilenameRight);
            rightDir.makeAbsolute();

            qDebug() << "leftDir exists? " << leftDir.exists();
            qDebug() << "rightDir exists? " << rightDir.exists();

            displayLogMessage("2D Imagepath = " + renderFilename + "\n" + "3D Left Imagepath = " + leftDir.absolutePath() + "\n" + leftDir.currentPath()  + "\n" + leftDir.dirName() + "\n" + leftDir.fromNativeSeparators(renderFilenameLeft) + "\n" + leftDir.homePath() + "\n" + leftDir.path()  + "\n" + leftDir.relativeFilePath(renderFilenameLeft) + "\n" + leftDir.toNativeSeparators(renderFilenameLeft) + "\n" + "3D Right Imagepath = " + renderFilenameRight + "\n", false);

            qDebug() << "leftDir.dirName()? " << leftDir.dirName();


            finishedRendering.render3D_left.load( leftDir.toNativeSeparators(renderFilenameLeft) );
            finishedRendering.render3D_right.load( rightDir.toNativeSeparators(renderFilenameRight) );

            //BEGIN
            //Redundant, but may be needed in future:

            while( finishedRendering.render3D_left.isNull() )
            {
                finishedRendering.render3D_left = QImage( leftDir.toNativeSeparators(renderFilenameLeft) ); //"../../left#####.jpg"
                if(QDateTime::currentMSecsSinceEpoch() > (startTime + 5000) )
                {
                    break;
                    displayLogMessage( tr("failed to add left image"), true );

                }
                QThread::msleep(1000);
            }
            startTime = QDateTime::currentMSecsSinceEpoch();
            while( finishedRendering.render3D_left.isNull() )
            {
                finishedRendering.render3D_right = QImage( rightDir.toNativeSeparators(renderFilenameRight) ); //"../../right#####.jpg"
                if(QDateTime::currentMSecsSinceEpoch() > (startTime + 5000) )
                {
                    break;
                    displayLogMessage( tr("failed to add right image"), true );

                }
                QThread::msleep(1000);
            }

            //END


            if(finishedRendering.render3D_left.isNull() || finishedRendering.render3D_right.isNull() )
            {
                displayLogMessage( tr("ERROR! Couldn't add stereo images to result!"), true );
            }

        }
        else
        {
            displayLogMessage( tr("ERROR! Couldn't read the stereo3D filepaths!"), true );
        }
        //finishedRendering.render3D_left = QImage("J:/Studium/B-ME6/Projektarbeit2/SVN/renderfarm/rf/branches/build-gsorf-Desktop_Qt_5_1_1_MinGW_32bit-Debug/left.0006.jpg");
        //finishedRendering.render3D_right = QImage("J:/Studium/B-ME6/Projektarbeit2/SVN/renderfarm/rf/branches/build-gsorf-Desktop_Qt_5_1_1_MinGW_32bit-Debug/right.0006.jpg");


        finishedRendering.is_stereo3D = true;
    }
    else
    {
        finishedRendering.render2D = QImage(renderFilename);
        finishedRendering.is_stereo3D = false;
    }

    qDebug() << "BlenderRenderEngine::createResult():end => Size of finishedRendering: " << sizeof(finishedRendering);

    emit sendResultBack(finishedRendering);

    quit();
}

void BlenderRenderEngine::loadRenderedImage()
{
    QPixmap image(renderFilename );

    emit updateClientGUIThumbnail(image);
}

void BlenderRenderEngine::setBlenderPath(QString path)
{
    blenderPath=path;
}

QString BlenderRenderEngine::getBlenderPath()
{
    return blenderPath;
}

void BlenderRenderEngine::quit()
{
    blender->close(); //Release the QProcess-Handle
    blender->kill(); //Kill a remaining Blender process

    if(!blenderError)
        loadRenderedImage();
    else
    {
        QString errormsg;
        if(blenderWarning)
        {
            errormsg = "Blender: WARNING!";
        }
        else
        {
            errormsg = "Blender: ERROR!";
            displayLogMessage( errormsg + "\nThe file could not be opened in Blender: " + blender->errorString(), true );
        }
    }

    blenderError = false;
    blenderWarning = false;

    emit updateClientRenderStatus(200);
}

void BlenderRenderEngine::displayLogMessage(const QString message, bool error)
{
    QString msg(message);
    if(error)
    {
        //QMessageBox::critical(NULL, "There is an error!", "The following error occured:\n\n" + msg);
        msg = "<span style=\"color:red; font-weight:bold;\">" + msg + "</span>";
        emit updateClientGUIConsole(msg);
    }
    else
    {
        emit updateClientGUIConsole(msg);
    }
}

/*
 *QProcess-Event-Callbacks:
 */

void BlenderRenderEngine::onEngineStarted()
{
    displayLogMessage("##########onEngineStarted", false);
}

void BlenderRenderEngine::onEngineBytesWritten(qint64)
{
    displayLogMessage("##########onEngineBytesWritten", false);
}

void BlenderRenderEngine::onEngineError(QProcess::ProcessError)
{
    displayLogMessage("onEngineError: The following error occured:\n\n" + blender->errorString(), true);

    //Something happened!
    //QMessageBox::critical(NULL, "There is an error!", "The following error occured:\n\n" + blender->errorString());

    if(blender->error() == QProcess::Crashed )
    {
        qDebug() << "QProcess::Crashed";
    }
    else if(blender->error() == QProcess::ReadError)
    {
        qDebug() << "QProcess::ReadError";
    }


    blenderError = true;
    quit();

}

void BlenderRenderEngine::onEngineFinished()
{
    displayLogMessage("##########onEngineFinished", false);

    blender->waitForFinished();
    createResult();
}

void BlenderRenderEngine::onEngineAboutToClose()
{
    displayLogMessage("##########onEngineAboutToClose", false);
}

void BlenderRenderEngine::onEngineReadChannelFinished()
{
    displayLogMessage("##########onEngineReadChannelFinished", false);
}

void BlenderRenderEngine::onEngineReadyRead()
{
    displayLogMessage("##########onEngineReadyRead", false);
    if(blender->canReadLine())
    {
        QByteArray result = blender->readAll();
        QString message = result;
        float current, max, percent;


        displayLogMessage( message, false);
        blender->readLine();

        /*** Check for errors ***/
        if(result.indexOf("Error") != -1)
        {
            if(result.indexOf("Cannot render, no camera") != -1)
                blenderError = true;
        }
        if(result.indexOf("Warning") != -1)
        {
            blenderWarning = true;
        }

        /*** Check for saved files ***/

        /* EXAMPLE: "Saved: /tmp/left.L0000.jpg
                     Saved: /tmp/right.R0000.jpg"
           EXAMPLE: "Saved: /home/grpc04/Desktop/renderfarm/rf/branches/build-gsorf-Desktop-Debug/awesomeRender_000000.jpg Time: 00:47.79 (Saving: 00:00.02)" */




        if(result.indexOf("Saved") != -1)
        {
            result.replace('\n', ' ');


            QList <QByteArray> stringparts = result.split(' ');
            message = "Blender seems to have saved a file: ";

            for(int i=0; i<stringparts.count(); i++)
            {
                if(i>0 && stringparts[i-1].indexOf("Saved:") != -1)
                {
                    message += "\n" + QString::number(i) + "\t" + stringparts.at(i);
                    if(stringparts.at(i).indexOf("left") != -1 && !render3dLeftSaved)
                    {
                        QDir renderDirLeft = QDir(stringparts.at(i));
                        renderFilenameLeft = renderDirLeft.absolutePath();
                        render3dLeftSaved = true;
                        message += "\n<span style=\"color: green ; font-weight: bold ;\">Render3D_left saved! (" + renderFilenameLeft + ")</span>\n";
                    }
                    else if(stringparts.at(i).indexOf("right") != -1 &&  !render3dRightSaved)
                    {
                        QDir renderDirRight = QDir(stringparts.at(i));
                        renderFilenameRight = renderDirRight.absolutePath();
                        render3dRightSaved = true;
                        message += "\n<span style=\"color: green ; font-weight: bold ;\">Render3D_right saved! (" + renderFilenameRight + ")</span>\n";
                    }
                }
            }

            displayLogMessage(message, false);
        }

        //EXAMPLE (Blender Render): Fra:1 Mem:42.92M (9.98M, peak 64.36M) | Scene, Part 3-135
        //EXAMPLE (Cycles  Render): Fra:0 Mem:36.83M (78.18M, peak 178.01M) | Mem: 146.19M, Peak: 146.19M | Scene, RenderLayer | Elapsed: 00:09.82 | Rendering | Path Tracing Tile 0/72

        //FALSE ALARM (Blender Render): Fra:4 Mem:15.37M (0.00M, Peak 15.56M) | Preparing Scene data


        if(result.indexOf("|") != -1) //if there is a separator
        {
            QTextStream in(&result);
            QString useless;
            QChar separator;

            if(result.indexOf("Path Tracing Tile") != -1) //if there is a separator
            {
                //CYCLES Render Engine
                result = result.right(result.size() - (result.indexOf("Path Tracing Tile") + 17) ); //Extract right part (from: "Path Tracing Tile"):

                in >> current; //get the first number
                in >> separator; //get the separator
                in >> max;     //get the second number
            }
            else
            {
                //BLENDER Render Engine
                result = result.right(result.size() - (result.indexOf("|") + 1) ); //Extract right part (from: "|"):

                in >> useless >> useless;  //get the text left (commas ignored!)
                in >> current; //get the first number
                in >> separator; //get the separator
                in >> max;     //get the second number
            }

        }
        else
        {
            return;
        }

        if(current == 0 || max == 0)
            return;
        else
            percent = qMax( ( (current / max) * 100) - 5, (float)0); //5 percent for creating the task, file and blender

        emit updateClientRenderStatus( percent + 5); //start with 5% progress
    }

}

void BlenderRenderEngine::onEngineDestroyed()
{
    displayLogMessage("##########onEngineDestroyed", false);
}

void BlenderRenderEngine::onSetFileCached(bool cached)
{
    //Set file cache and DON'T take the Task-File, if cached
    taskCached = cached;
    if(cached)
    {
        qDebug("BlenderRenderEngine::onSetFileCached(bool cached) -> cached: true");

    }
    else
    {
        qDebug("BlenderRenderEngine::onSetFileCached(bool cached) -> cached: false");

    }
}

