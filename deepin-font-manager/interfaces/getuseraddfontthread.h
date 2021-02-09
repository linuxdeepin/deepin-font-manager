#ifndef GETUSERADDFONTTHREAD_H
#define GETUSERADDFONTTHREAD_H

#include "dfontinfomanager.h"

#include <QThread>

class GetUserAddFontThread : public QThread
{
    Q_OBJECT

public:
    explicit GetUserAddFontThread(QObject *parent = nullptr);

protected:
    void run();
};

#endif // GETUSERADDFONTTHREAD_H
