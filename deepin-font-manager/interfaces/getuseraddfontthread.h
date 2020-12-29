#ifndef GETUSERADDFONTTHREAD_H
#define GETUSERADDFONTTHREAD_H

#include "dfontinfomanager.h"

#include <QThread>

class GetUserAddFontThread : public QThread
{
    Q_OBJECT

public:
    GetUserAddFontThread();

protected:
    void run();

signals:
    void onThreadFinished(QList<DFontInfo> &list);

};

#endif // GETUSERADDFONTTHREAD_H
