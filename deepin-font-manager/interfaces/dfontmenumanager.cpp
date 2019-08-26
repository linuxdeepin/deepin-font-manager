#include "interfaces/dfontmenumanager.h"

#include <DLog>

DFontMenuManager *DFontMenuManager::instance = nullptr;

DFontMenuManager::DFontMenuManager(QObject *parent)
    : QObject(parent)
{
    initMenuData();
}

DFontMenuManager *DFontMenuManager::getInstance()
{
    if (nullptr == instance) {
        instance = new DFontMenuManager();
    }

    return instance;
}

void DFontMenuManager::initMenuData()
{
    // ToDo:
    //    Need to localize the menu string

    // Tools bar menu & Right key menu.
    m_fontToolsBarMenus.insert(MenuAction::M_AddFont,
                               new FMenuItem("添加字体", MenuAction::M_AddFont));

    FMenuItem *themeMenus = new FMenuItem("主题", MenuAction::M_Theme, true);
    themeMenus->subMenulist.push_back(new FMenuItem("浅色主体", MenuAction::M_ThemeLight));
    themeMenus->subMenulist.push_back(new FMenuItem("深色主体", MenuAction::M_ThemeDark));
    themeMenus->subMenulist.push_back(new FMenuItem("跟随系统", MenuAction::M_ThemeFollowSystem));
    m_fontToolsBarMenus.insert(MenuAction::M_Theme, themeMenus);
    m_fontToolsBarMenus.insert(MenuAction::M_Help, new FMenuItem("帮助", MenuAction ::M_Help));

    // Right key menu data
    m_fontRightKeyMenus.insert(MenuAction::M_AddFont,
                               new FMenuItem("添加字体", MenuAction::M_AddFont));

    m_fontRightKeyMenus.insert(MenuAction::M_Separator, new FMenuItem("", MenuAction::M_Separator));

    m_fontRightKeyMenus.insert(MenuAction::M_EnableOrDisable,
                               new FMenuItem("启用字体", MenuAction::M_EnableOrDisable));
    m_fontRightKeyMenus.insert(MenuAction::M_DeleteFont,
                               new FMenuItem("删除字体", MenuAction::M_DeleteFont));
    m_fontRightKeyMenus.insert(MenuAction::M_Faverator,
                               new FMenuItem("收藏", MenuAction::M_Faverator));

    m_fontRightKeyMenus.insert(MenuAction::M_Separator, new FMenuItem("", MenuAction::M_Separator));

    m_fontRightKeyMenus.insert(MenuAction::M_FontInfo,
                               new FMenuItem("显示信息", MenuAction::M_FontInfo));

    m_fontRightKeyMenus.insert(MenuAction::M_Separator, new FMenuItem("", MenuAction::M_Separator));

    m_fontRightKeyMenus.insert(MenuAction::M_ShowFontPostion,
                               new FMenuItem("在文件管理器中显示", MenuAction::M_ShowFontPostion));

    qDebug() << " font menu size = " << m_fontToolsBarMenus.size();
}

QMenu *DFontMenuManager::createToolBarSettingsMenu(FMenuActionTriggle actionTriggle)
{
    Q_UNUSED(actionTriggle);

    QMenu *mainMenu = new QMenu();

    QMap<MenuAction, FMenuItem *>::const_iterator it = m_fontToolsBarMenus.constBegin();
    for (; it != m_fontToolsBarMenus.constEnd(); it++) {
        QAction *newAction = nullptr;

        if (it.key() == MenuAction::M_Separator) {
            mainMenu->addSeparator();
            continue;
        }

        if (!it.value()->fHaveSubMenu) {
            newAction = mainMenu->addAction(it.value()->actionName);
            newAction->setData(it.key());
            it.value()->action = newAction;
        } else {
            QMenu *subMenu = mainMenu->addMenu(it.value()->actionName);

            QVector<FMenuItem *>::Iterator iter = it.value()->subMenulist.begin();

            for (; iter != it.value()->subMenulist.end(); iter++) {
                newAction = subMenu->addAction((*iter)->actionName);
                newAction->setData((*iter)->actionId);
                (*iter)->action = newAction;
            }
        }
    }

    return mainMenu;
}

QMenu *DFontMenuManager::createRightKeyMenu(FMenuActionTriggle actionTriggle)
{
    Q_UNUSED(actionTriggle);

    QMenu *rightKeyMenu = new QMenu();

    QMap<MenuAction, FMenuItem *>::const_iterator it = m_fontRightKeyMenus.constBegin();
    for (; it != m_fontRightKeyMenus.constEnd(); it++) {
        QAction *newAction = nullptr;

        if (it.key() == MenuAction::M_Separator) {
            rightKeyMenu->addSeparator();
            continue;
        }

        if (!it.value()->fHaveSubMenu) {
            newAction = rightKeyMenu->addAction(it.value()->actionName);
            newAction->setData(it.key());
            it.value()->action = newAction;
        } else {
            QMenu *subMenu = rightKeyMenu->addMenu(it.value()->actionName);

            QVector<FMenuItem *>::Iterator iter = it.value()->subMenulist.begin();

            for (; iter != it.value()->subMenulist.end(); iter++) {
                newAction = subMenu->addAction((*iter)->actionName);
                newAction->setData((*iter)->actionId);
                (*iter)->action = newAction;
            }
        }
    }

    return rightKeyMenu;
}

QAction *DFontMenuManager::getActionByMenuAction(MenuAction maction, MenuType menuType)
{
    QAction *action = nullptr;

    if (MenuType::ToolBarMenu == menuType) {
        auto it = m_fontToolsBarMenus.find(maction);
        if (it != m_fontToolsBarMenus.end()) {
            action = it.value()->action;
        }
    } else if (MenuType::RightKeyMenu == menuType) {
        auto it = m_fontRightKeyMenus.find(maction);
        if (it != m_fontRightKeyMenus.end()) {
            action = it.value()->action;
        }
    } else {
        qDebug() << __FUNCTION__ << " Unknow menu type = " << menuType
                 << " for MenuAction=" << maction;
    }

    return action;
}
