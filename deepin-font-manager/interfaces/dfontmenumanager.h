#ifndef DFONTMENUMANAGER_H
#define DFONTMENUMANAGER_H
#include "dfontpreviewitemdef.h"

#include <QAction>
#include <QActionGroup>
#include <QObject>
#include <QSet>
#include <QVector>

#include <DMenu>

DWIDGET_USE_NAMESPACE

struct DFontPreviewItemData;
class DFontMenuManager : public QObject
{
    Q_OBJECT
public:
    explicit DFontMenuManager(QObject *parent = nullptr);

    static DFontMenuManager *getInstance();

    enum MenuType {
        ToolBarMenu,
        RightKeyMenu,
    };

    enum MenuAction {
        M_Separator,
        M_AddFont,
        M_Help,
        M_EnableOrDisable,
        M_DeleteFont,
        M_Faverator,
        M_FontInfo,
        M_ShowFontPostion,
    };

    struct FMenuItem {
        FMenuItem(QString name, MenuAction id, bool haveSubMenu = false, bool group = false)
            : actionName(name)
            , actionId(id)
            , fHaveSubMenu(haveSubMenu)
            , fGroupSubMenu(group)
        {
        }

        QString actionName;
        MenuAction actionId;
        QAction *action {nullptr};
        bool fHaveSubMenu {false};
        bool fGroupSubMenu {false};
        QActionGroup *actionGroup {nullptr};
        QVector<FMenuItem *> subMenulist;
    };

    typedef void (*FMenuActionTriggle)(QAction *);

    DMenu *createToolBarSettingsMenu(FMenuActionTriggle actionTriggle = nullptr);
    DMenu *createRightKeyMenu(FMenuActionTriggle actionTriggle = nullptr);

    QAction *getActionByMenuAction(MenuAction maction, MenuType menuType);

private:
    void initMenuData();

    QMap<MenuAction, FMenuItem *> m_fontToolsBarMenus;
    QMap<MenuAction, FMenuItem *> m_fontRightKeyMenus;

    QVector<FMenuItem *> m_fontToolBarMenuData;
    QVector<FMenuItem *> m_fontRightMenuData;

private:
    static DFontMenuManager *instance;
signals:

public slots:
    void onRightKeyMenuPopup(DFontPreviewItemData fontData);
};

#endif  // DFONTMENUMANAGER_H
