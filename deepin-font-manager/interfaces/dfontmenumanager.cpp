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
    m_fontToolBarMenuData.push_back(new FMenuItem("添加字体", MenuAction::M_AddFont));
    FMenuItem *themeMenus = new FMenuItem("主题", MenuAction::M_Theme, true, true);
    themeMenus->subMenulist.push_back(new FMenuItem("浅色主体", MenuAction::M_ThemeLight));
    themeMenus->subMenulist.push_back(new FMenuItem("深色主体", MenuAction::M_ThemeDark));
    themeMenus->subMenulist.push_back(new FMenuItem("跟随系统", MenuAction::M_ThemeFollowSystem));
    m_fontToolBarMenuData.push_back(themeMenus);
    m_fontToolBarMenuData.push_back(new FMenuItem("帮助", MenuAction ::M_Help));
    m_fontToolBarMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));

    // Right key menu data
    m_fontRightMenuData.push_back(new FMenuItem("添加字体", MenuAction::M_AddFont));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(new FMenuItem("启用字体", MenuAction::M_EnableOrDisable));
    m_fontRightMenuData.push_back(new FMenuItem("删除字体", MenuAction::M_DeleteFont));
    m_fontRightMenuData.push_back(new FMenuItem("收藏", MenuAction::M_Faverator));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(new FMenuItem("显示信息", MenuAction::M_FontInfo));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(new FMenuItem("在文件管理器中显示", MenuAction::M_ShowFontPostion));
}

QMenu *DFontMenuManager::createToolBarSettingsMenu(FMenuActionTriggle actionTriggle)
{
    Q_UNUSED(actionTriggle);

    QMenu *mainMenu = new QMenu();

    for (auto it : m_fontToolBarMenuData) {
        QAction *newAction = nullptr;

        if (it->actionId == MenuAction::M_Separator) {
            mainMenu->addSeparator();
            continue;
        }

        if (!it->fHaveSubMenu) {
            newAction = mainMenu->addAction(it->actionName);
            newAction->setData(it->actionId);
            it->action = newAction;

            m_fontToolsBarMenus.insert(it->actionId, it);
        } else {
            QMenu *subMenu = mainMenu->addMenu(it->actionName);

            // Create a action group for group menu
            if (it->fGroupSubMenu) {
                it->actionGroup = new QActionGroup(subMenu);
            }

            for (auto iter : it->subMenulist) {
                newAction = subMenu->addAction(iter->actionName);
                newAction->setData(iter->actionId);
                iter->action = newAction;

                if (it->fGroupSubMenu) {
                    newAction->setCheckable(true);
                    it->actionGroup->addAction(newAction);
                }

                m_fontToolsBarMenus.insert(iter->actionId, iter);
            }
        }
    }

    return mainMenu;
}

QMenu *DFontMenuManager::createRightKeyMenu(FMenuActionTriggle actionTriggle)
{
    Q_UNUSED(actionTriggle);

    QMenu *rightKeyMenu = new QMenu();

    for (auto it : m_fontRightMenuData) {
        QAction *newAction = nullptr;

        if (it->actionId == MenuAction::M_Separator) {
            rightKeyMenu->addSeparator();
            continue;
        }

        if (!it->fHaveSubMenu) {
            newAction = rightKeyMenu->addAction(it->actionName);
            newAction->setData(it->actionId);
            it->action = newAction;

            m_fontRightKeyMenus.insert(it->actionId, it);
        } else {
            QMenu *subMenu = rightKeyMenu->addMenu(it->actionName);

            for (auto iter : it->subMenulist) {
                newAction = subMenu->addAction(iter->actionName);
                newAction->setData(iter->actionId);
                iter->action = newAction;

                m_fontRightKeyMenus.insert(iter->actionId, iter);
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
        qDebug() << __FUNCTION__ << " Unknow menu type = " << menuType << " for MenuAction=" << maction;
    }

    return action;
}
