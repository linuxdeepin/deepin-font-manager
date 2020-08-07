#include "interfaces/dfontmenumanager.h"
#include <DApplicationHelper>
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
        new FMenuItem(DApplication::translate("Menu", "Add font"), MenuAction::M_AddFont));
    m_fontToolBarMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));

    // Right key menu data
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Add font"), MenuAction::M_AddFont));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Enable"), MenuAction::M_EnableOrDisable));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Delete"), MenuAction::M_DeleteFont));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Export"), MenuAction::M_ExportFont));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Favorite"), MenuAction::M_Faverator));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Details"), MenuAction::M_FontInfo));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Display in file manager"), MenuAction::M_ShowFontPostion));
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

void DFontMenuManager::onRightKeyMenuPopup(const DFontPreviewItemData &itemData, bool hasUser, bool enableDisable, bool hasCurFont)
{
    // Disable delete menu for system font
    QAction *delAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                             DFontMenuManager::M_DeleteFont, DFontMenuManager::MenuType::RightKeyMenu);

    QAction *exportAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                                DFontMenuManager::M_ExportFont, DFontMenuManager::MenuType::RightKeyMenu);

    QAction *faveriteAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                                  DFontMenuManager::M_Faverator, DFontMenuManager::MenuType::RightKeyMenu);

    QAction *enableOrDisableAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                                         DFontMenuManager::M_EnableOrDisable, DFontMenuManager::MenuType::RightKeyMenu);

    // Disable delete menu on system font
    if (nullptr != delAction && hasUser) {
        delAction->setDisabled(false);
    } else {
        delAction->setDisabled(true);
    }

    // Export menu on system font
    if (nullptr != exportAction && (hasUser || hasCurFont)) {
        exportAction->setDisabled(false);
    } else {
        exportAction->setDisabled(true);
    }

    // Favarite font Menu
    if (nullptr != faveriteAction && itemData.fontData.isCollected()) {
        faveriteAction->setText(DApplication::translate("Menu", "Unfavorite"));
    } else {
        faveriteAction->setText(DApplication::translate("Menu", "Favorite"));
    }

    // Enable/Disable Menu
    if (nullptr != enableOrDisableAction && itemData.fontData.isEnabled()) {
        if (enableDisable) {
            enableOrDisableAction->setEnabled(true);
        } else {
            enableOrDisableAction->setEnabled(false);
        }
        enableOrDisableAction->setText(DApplication::translate("Menu", "Disable"));
    } else {
        enableOrDisableAction->setEnabled(true);
        enableOrDisableAction->setText(DApplication::translate("Menu", "Enable"));
    }
}
