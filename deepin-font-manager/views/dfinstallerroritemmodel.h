#ifndef DFINSTALLERRORITEMMODEL_H
#define DFINSTALLERRORITEMMODEL_H

#include "globaldef.h"

#include <QMetaType>
#include <QString>

#include <DCheckBox>
#include <DLabel>

struct DFInstallErrorItemModel
{
    bool bChecked;
    bool bSelectable;
    QString strFontFileName;
    QString strFontFilePath;
    QString strFontInstallStatus;

    DFInstallErrorItemModel()
    {
        bChecked = false;
        bSelectable = true;
        strFontFileName = "";
        strFontFilePath = "";
        strFontInstallStatus = "";
    }
};

Q_DECLARE_METATYPE(DFInstallErrorItemModel)

#endif  // DFINSTALLERRORITEMMODEL_H
