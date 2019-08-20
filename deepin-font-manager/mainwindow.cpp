/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "utils.h"
#include "dtitlebar.h"
#include "dhidpihelper.h"
#include "dthememanager.h"

#include <QSvgWidget>
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent),
      m_mainWidget(new QWidget),
      m_mainLayout(new QStackedLayout(m_mainWidget)),
      m_homePage(new HomePage),
      m_singleFilePage(new SingleFilePage),
      m_multiFilePage(new MultiFilePage),
      m_settings("deepin", "deepin-font-installer"),
      m_themeAction(new QAction(tr("Dark theme"), this))
{
    titlebar()->setIcon(QIcon(":/images/icon.svg"));
    titlebar()->setTitle("");
#if DTK_VERSION >= 0x02000600
    titlebar()->setBackgroundTransparent(true);
#endif

    // add widget to main layout.
    m_mainLayout->addWidget(m_homePage);
    m_mainLayout->addWidget(m_singleFilePage);
    m_mainLayout->addWidget(m_multiFilePage);

    // init window flags.
    setWindowTitle(tr("Deepin Font Installer"));
    setCentralWidget(m_mainWidget);
    setAcceptDrops(true);

    // add menu to titlebar.
    QMenu *menu = new QMenu;
    menu->addAction(m_themeAction);
    menu->addSeparator();
    titlebar()->setMenu(menu);

    // init theme action.
    m_themeAction->setCheckable(true);

    // init settings.
    if (!m_settings.contains("darkTheme")) {
        m_settings.setValue("darkTheme", false);
    }

    // init theme.
    initTheme();

    // connect the signals to the slot function.
    connect(m_homePage, &HomePage::fileSelected, this, &MainWindow::onSelected);
    connect(m_multiFilePage, &MultiFilePage::countChanged, this, &MainWindow::refreshPage);
    connect(m_multiFilePage, &MultiFilePage::installing, this, &MainWindow::setDisable);
    connect(m_multiFilePage, &MultiFilePage::installFinished, this, &MainWindow::setEnable);
    connect(m_themeAction, &QAction::triggered, this, &MainWindow::switchTheme);
}

MainWindow::~MainWindow()
{
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();

    // not has urls.
    if (!mime->hasUrls()) {
        return e->ignore();
    }

    // traverse.
    for (const auto &url : mime->urls()) {
        const QFileInfo info(url.toLocalFile());

        if (info.isDir() || Utils::isFontMimeType(url.toLocalFile())) {
            m_homePage->setIconPixmap(true);
            return e->accept();
        }
    }

    e->ignore();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *e)
{
    m_homePage->setIconPixmap(false);

    DMainWindow::dragLeaveEvent(e);
}

void MainWindow::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();

    if (!mime->hasUrls())
        return e->ignore();

    e->accept();

    // find font files.
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        const QString localPath = url.toLocalFile();
        const QFileInfo info(localPath);

        if (info.isFile() && Utils::isFontMimeType(localPath)) {
            fileList << localPath;
        } else if (info.isDir()) {
            for (const auto &file : QDir(localPath).entryInfoList()) {
                if (Utils::isFontMimeType(file.absoluteFilePath())) {
                    fileList << file.absoluteFilePath();
                }
            }
        }
    }

    m_homePage->setIconPixmap(false);
    onSelected(fileList);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void MainWindow::initTheme()
{
    const bool isDarkTheme = m_settings.value("darkTheme").toBool();

    if (isDarkTheme) {
        DThemeManager::instance()->setTheme("dark");
        m_themeAction->setChecked(true);
        setStyleSheet(Utils::getQssContent(":/qss/dark.qss"));
    } else {
        DThemeManager::instance()->setTheme("light");
        m_themeAction->setChecked(false);
        setStyleSheet(Utils::getQssContent(":/qss/light.qss"));
    }
}

void MainWindow::switchTheme()
{
    const bool isDarkTheme = m_settings.value("darkTheme").toBool();

    if (isDarkTheme) {
        m_settings.setValue("darkTheme", false);
    } else {
        m_settings.setValue("darkTheme", true);
    }

    initTheme();
}

void MainWindow::setEnable()
{
    setAcceptDrops(true);

    // enable titlebar buttons.
    titlebar()->setDisableFlags(Qt::Widget);
}

void MainWindow::setDisable()
{
    setAcceptDrops(false);

    // disable titlebar buttons.
    titlebar()->setDisableFlags(Qt::WindowMinimizeButtonHint
                                | Qt::WindowCloseButtonHint
                                | Qt::WindowMaximizeButtonHint
                                | Qt::WindowSystemMenuHint);
}

void MainWindow::refreshPage()
{
    const int count = m_multiFilePage->getInfoList().count();

    if (count == 0)
        return;

    if (count == 1) {
        // switch to single file page.
        m_mainLayout->setCurrentIndex(1);
        // update info.
        m_singleFilePage->updateInfo(m_multiFilePage->getInfoList().first());
        titlebar()->setTitle("");
    } else {
        // switch to multi file page.
        m_mainLayout->setCurrentIndex(2);
        titlebar()->setTitle(tr("Bulk Install"));
    }
}

void MainWindow::onSelected(const QStringList &files)
{
    // add file path to multiFilePage class.
    m_multiFilePage->addItems(files);
    refreshPage();
}
