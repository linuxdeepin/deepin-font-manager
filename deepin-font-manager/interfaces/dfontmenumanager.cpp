#include "interfaces/dfontmenumanager.h"

#include <DApplication>
#include <DLog>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

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
    m_fontToolBarMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Add Fonts"), MenuAction::M_AddFont));
    m_fontToolBarMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Help"), MenuAction ::M_Help));

    m_fontToolBarMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));

    // Right key menu data
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Add Fonts"), MenuAction::M_AddFont));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Enable Font"), MenuAction::M_EnableOrDisable));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Delete Font"), MenuAction::M_DeleteFont));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Add Favorite"), MenuAction::M_Faverator));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Font Detail"), MenuAction::M_FontInfo));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Font File Position"), MenuAction::M_ShowFontPostion));
}

QMenu *DFontMenuManager::createToolBarSettingsMenu(FMenuActionTriggle actionTriggle)
{
    Q_UNUSED(actionTriggle);

    DMenu *mainMenu = new DMenu();

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
            DMenu *subMenu = mainMenu->addMenu(it->actionName);

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

    DMenu *rightKeyMenu = new DMenu();

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
            DMenu *subMenu = rightKeyMenu->addMenu(it->actionName);

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

void DFontMenuManager::onRightKeyMenuPopup(DFontPreviewItemData fontData)
{
    // Disable delete menu for system font
    QAction *delAction = DFontMenuManager::getInstance()->getActionByMenuAction(
        DFontMenuManager::M_DeleteFont, DFontMenuManager::MenuType::RightKeyMenu);

    QAction *faveriteAction = DFontMenuManager::getInstance()->getActionByMenuAction(
        DFontMenuManager::M_Faverator, DFontMenuManager::MenuType::RightKeyMenu);

    QAction *enableOrDisableAction = DFontMenuManager::getInstance()->getActionByMenuAction(
        DFontMenuManager::M_EnableOrDisable, DFontMenuManager::MenuType::RightKeyMenu);

    // Disable delete menu on system font
    if (nullptr != delAction && fontData.fontInfo.isSystemFont) {
        delAction->setDisabled(true);
    } else {
        delAction->setDisabled(false);
    }

    // Favarite font Menu
    if (nullptr != faveriteAction && fontData.isCollected) {
        faveriteAction->setText(DApplication::translate("Menu", "Remove Favorite"));
    } else {
        faveriteAction->setText(DApplication::translate("Menu", "Add Favorite"));
    }

    // Enable/Disable Menu
    if (nullptr != enableOrDisableAction && fontData.isEnabled) {
        enableOrDisableAction->setText(DApplication::translate("Menu", "Disable Font"));
    } else {
        enableOrDisableAction->setText(DApplication::translate("Menu", "Enable Font"));
    }
}
