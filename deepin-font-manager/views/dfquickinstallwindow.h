/*
 *
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:
*
* Maintainer:
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

#ifndef DFQUICKINSTALL_H
#define DFQUICKINSTALL_H

#include <DApplication>
#include <DComboBox>
#include <DLabel>
#include <DMainWindow>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class DFontInfoManager;
class FontManagerCore;
struct DFontInfo;
class DFontPreviewer;
class DFMDBManager;

class DFQuickInstallWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit DFQuickInstallWindow(const QStringList &files = QStringList(), QWidget *parent = nullptr);

    ~DFQuickInstallWindow() override;

    static constexpr int DEFAULT_WINDOW_W = 480;
    static constexpr int DEFAULT_WINDOW_H = 380;

protected:
    void initUI();
    void initConnections();
    void InitPreviewFont(DFontInfo &fontInfo);
    void installFont(const QStringList &files);

    void resizeEvent(QResizeEvent *event) override;
signals:
    void fileSelected(const QStringList &fileList);
    void quickInstall();
    void requestShowMainWindow(const QStringList &fileList);

public slots:
    void onFileSelected(const QStringList &fileList);
    void onInstallBtnClicked();
    void onFontInstallFinished();

private:
    DLabel *m_titleLabel {nullptr};
    QWidget *m_mainFrame {nullptr};

    DComboBox *m_fontType {nullptr};
    DFontPreviewer *m_fontPreviewTxt {nullptr};

    DLabel *m_stateLabel {nullptr};
    DPushButton *m_actionBtn {nullptr};

    DFontInfoManager *m_fontInfoManager;
    FontManagerCore *m_fontManager;
    DFMDBManager *m_dbManager;


    QStringList m_installFiles;

    DPalette m_oldPaStateLbl;
};

#endif  // DFQUICKINSTALL_H
