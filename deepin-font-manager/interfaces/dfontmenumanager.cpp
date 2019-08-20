#include "interfaces/dfontmenumanager.h"
#include <DLog>

DFontMenuManager *DFontMenuManager::instance = nullptr;

DFontMenuManager::DFontMenuManager(QObject *parent)
    : QObject(parent)
{
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
    //    Need localize the menu string
    // fontMenus.
    fontMenus.insert(MenuAction::M_AddFont, new FMenuItem("添加字体", MenuAction::M_AddFont));

    FMenuItem *themeMenus = new FMenuItem("主题", MenuAction::M_Theme, true);
    themeMenus->subMenulist.push_back(new FMenuItem("浅色主体", MenuAction::M_ThemeLight));
    themeMenus->subMenulist.push_back(new FMenuItem("深色主体", MenuAction::M_ThemeDark));
    themeMenus->subMenulist.push_back(new FMenuItem("跟随系统", MenuAction::M_ThemeFollowSystem));
    fontMenus.insert(MenuAction::M_Theme, themeMenus);
    fontMenus.insert(MenuAction::M_Help, new FMenuItem("帮助", MenuAction ::M_Help));

    qDebug() << " font menu size = " << fontMenus.size();
}

QMenu *DFontMenuManager::createToolBarSettingsMenu(FMenuActionTriggle actionTriggle)
{
    initMenuData();
    QMenu *mainMenu = new QMenu();

    QMap<MenuAction, FMenuItem *>::const_iterator it = fontMenus.constBegin();
    for (; it != fontMenus.constEnd(); it++) {
        if (!it.value()->fHaveSubMenu) {
            mainMenu->addAction(it.value()->actionName)->setData(it.key());
        } else {
            QMenu *subMenu = mainMenu->addMenu(it.value()->actionName);

            QVector<FMenuItem *>::Iterator iter = it.value()->subMenulist.begin();

            for (; iter != it.value()->subMenulist.end(); iter++) {
                subMenu->addAction((*iter)->actionName)->setData((*iter)->actionId);
            }
        }
    }

    return mainMenu;
}
