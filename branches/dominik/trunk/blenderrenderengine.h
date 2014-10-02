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
