#include "dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.setWindowFlags(Qt::WindowCloseButtonHint);
    w.show();

    return a.exec();
}

//Merge Protocol Client & Server:
//Client code in: dialog, task_client, updmanager, blenderrenderengine
//Server code in: dialog, task


/*
99 little bugs in the code
99 little bugs in the code
Take one down, patch it around
117 little bugs in the code
*/
