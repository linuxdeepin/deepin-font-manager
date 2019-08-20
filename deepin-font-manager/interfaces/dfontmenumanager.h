#ifndef DFONTMENUMANAGER_H
#define DFONTMENUMANAGER_H

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QSet>
#include <QVector>
class DFontMenuManager : public QObject
{
    Q_OBJECT
public:
    explicit DFontMenuManager(QObject *parent = nullptr);

    static DFontMenuManager *getInstance();

    enum MenuAction {
        M_Separator,
        M_AddFont,
        M_Theme,
        M_ThemeLight,
        M_ThemeDark,
        M_ThemeFollowSystem,
        M_Help,
    };

    struct FMenuItem {
        FMenuItem(QString name, MenuAction id, bool haveSubMenu = false)
            : actionName(name)
            , actionId(id)
            , fHaveSubMenu(haveSubMenu)
        {
        }

        QString actionName;
        MenuAction actionId;
        bool fHaveSubMenu {false};
        QVector<FMenuItem *> subMenulist;
    };

    typedef void (*FMenuActionTriggle)(QAction *);

    QMenu *createToolBarSettingsMenu(FMenuActionTriggle actionTriggle = nullptr);

private:
    void initMenuData();

    QMap<MenuAction, FMenuItem *> fontMenus;

private:
    static DFontMenuManager *instance;
signals:

public slots:
};

#endif  // DFONTMENUMANAGER_H
