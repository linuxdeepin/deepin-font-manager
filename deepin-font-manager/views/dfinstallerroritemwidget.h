#ifndef DFINSTALLERRORITEMWIDGET_H
#define DFINSTALLERRORITEMWIDGET_H

#include "globaldef.h"

#include <DCheckBox>
#include <DLabel>
#include <QString>

DWIDGET_USE_NAMESPACE

class DFInstallErrorItemModel
{
public:
    bool bChecked;
    bool bSelectable;
    QString strFontFileName;
    QString strFontFilePath;
    QString strFontInstallStatus;

    DFInstallErrorItemModel();
};

class DFInstallErrorItemWidget : public QWidget
{
    Q_OBJECT
public:
    DFInstallErrorItemWidget(DFInstallErrorItemModel *itemModel);

    void initUI();

    DFInstallErrorItemModel *m_itemModel;

    DCheckBox *m_chooseCheck;
    DLabel *m_fontFileNameLabel;
    DLabel *m_fontInstallStatusLabel;
};

#endif  // DFINSTALLERRORITEMWIDGET_H
