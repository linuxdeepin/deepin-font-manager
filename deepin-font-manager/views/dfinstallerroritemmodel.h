#ifndef DFINSTALLERRORITEMMODEL_H
#define DFINSTALLERRORITEMMODEL_H

#include <QMetaType>
#include <QString>


struct DFInstallErrorItemModel {
    bool bChecked;
    bool bSelectable;
    bool bSystemFont;
    QString strFontFileName;
    QString strFontFilePath;
    QString strFontInstallStatus;

    DFInstallErrorItemModel()
    {
        bChecked = false;
        bSelectable = true;
        bSystemFont = false;
        strFontFileName = "";
        strFontFilePath = "";
        strFontInstallStatus = "";
    }
};

Q_DECLARE_METATYPE(DFInstallErrorItemModel)

#endif  // DFINSTALLERRORITEMMODEL_H
