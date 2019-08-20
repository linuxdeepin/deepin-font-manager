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

#include "singlefilepage.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTextLayout>
#include <QSvgWidget>
#include <QFileInfo>
#include <QUrlQuery>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QDir>

const QString holdTextInRect(const QFont &font, QString text, const QSize &size)
{
    QFontMetrics fm(font);
    QTextLayout layout(text);

    layout.setFont(font);

    QStringList lines;
    QTextOption &text_option = *const_cast<QTextOption*>(&layout.textOption());

    text_option.setWrapMode(QTextOption::WordWrap);
    text_option.setAlignment(Qt::AlignTop | Qt::AlignLeft);

    layout.beginLayout();

    QTextLine line = layout.createLine();
    int height = 0;
    int lineHeight = fm.height();

    while (line.isValid()) {
        height += lineHeight;

        if (height + lineHeight > lineHeight * 2) {
            const QString &end_str = fm.elidedText(text.mid(line.textStart()), Qt::ElideRight, size.width());

            layout.endLayout();
            layout.setText(end_str);

            text_option.setWrapMode(QTextOption::NoWrap);
            layout.beginLayout();
            line = layout.createLine();
            line.setLineWidth(size.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        lines.append(text.mid(line.textStart(), line.textLength()));

        if (height + lineHeight > lineHeight * 2)
            break;

        line = layout.createLine();
    }

    layout.endLayout();

    return lines.join("");
}

SingleFilePage::SingleFilePage(QWidget *parent)
    : QWidget(parent),
      m_fontInfo(nullptr),
      m_fontInfoManager(DFontInfoManager::instance()),
      m_fontManager(DFontManager::instance()),
      m_nameLabel(new QLabel),
      m_styleLabel(new QLabel),
      m_typeLabel(new QLabel),
      m_versionLabel(new QLabel),
      m_copyrightLabel(new QLabel),
      m_descriptionLabel(new QLabel),
      m_tipsLabel(new QLabel()),
      m_installBtn(new DSuggestButton),
      m_uninstallBtn(new QPushButton(tr("Remove"))),
      m_reinstallBtn(new QPushButton(tr("Reinstall"))),
      m_viewFileBtn(new DSuggestButton),
      m_closeBtn(new DSuggestButton),
      m_spinner(new DSpinner),
      m_bottomLayout(new QStackedLayout)
{
    QSvgWidget *iconWidget = new QSvgWidget(":/images/font-x-generic.svg");
    iconWidget->setFixedSize(70, 70);

    QLabel *styleLabel = new QLabel(tr("Style: "));
    QLabel *typeLabel = new QLabel(tr("Type: "));
    QLabel *versionLabel = new QLabel(tr("Version: "));
    QLabel *copyrightLabel = new QLabel(tr("Copyright: "));
    QLabel *descLabel = new QLabel(tr("Description: "));

    copyrightLabel->setAlignment(Qt::AlignTop);
    descLabel->setAlignment(Qt::AlignTop);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(styleLabel, m_styleLabel);
    formLayout->addRow(typeLabel, m_typeLabel);
    formLayout->addRow(versionLabel, m_versionLabel);
    formLayout->addRow(copyrightLabel, m_copyrightLabel);
    formLayout->addRow(descLabel, m_descriptionLabel);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setHorizontalSpacing(5);
    formLayout->setVerticalSpacing(7);

    QWidget *btnsWidget = new QWidget;
    QHBoxLayout *btnsLayout = new QHBoxLayout(btnsWidget);
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installBtn);
    btnsLayout->addWidget(m_uninstallBtn);
    btnsLayout->addWidget(m_reinstallBtn);
    btnsLayout->addWidget(m_viewFileBtn);
    btnsLayout->addWidget(m_closeBtn);
    btnsLayout->addStretch();
    btnsLayout->setSpacing(20);
    btnsLayout->setMargin(0);

    QWidget *progressWidget = new QWidget;
    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->addWidget(m_spinner, 0, Qt::AlignCenter);
    progressWidget->setFixedHeight(40);
    m_spinner->setFixedSize(30, 30);

    m_bottomLayout->addWidget(btnsWidget);
    m_bottomLayout->addWidget(progressWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(iconWidget, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_tipsLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(m_bottomLayout);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(50, 0, 50, 20);

    // init property.
    styleLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");
    typeLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");
    versionLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");
    copyrightLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");
    descLabel->setStyleSheet("QLabel { color: #797979; font-size: 14px; }");

    m_nameLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: 510; }");
    m_styleLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_typeLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_versionLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_copyrightLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_descriptionLabel->setStyleSheet("QLabel { font-size: 14px; }");

    m_copyrightLabel->setAlignment(Qt::AlignTop);
    m_copyrightLabel->setWordWrap(true);

    m_descriptionLabel->setAlignment(Qt::AlignTop);
    m_descriptionLabel->setWordWrap(true);
    m_tipsLabel->setText("");

    m_installBtn->setText(tr("Install"));
    m_installBtn->setFixedSize(160, 36);
    m_installBtn->setFocusPolicy(Qt::NoFocus);
    m_installBtn->setVisible(false);

    m_uninstallBtn->setFixedSize(160, 36);
    m_uninstallBtn->setFocusPolicy(Qt::NoFocus);
    m_uninstallBtn->setObjectName("GrayButton");
    m_uninstallBtn->setVisible(false);

    m_reinstallBtn->setFixedSize(160, 36);
    m_reinstallBtn->setFocusPolicy(Qt::NoFocus);
    m_reinstallBtn->setObjectName("GrayButton");
    m_reinstallBtn->setVisible(false);

    m_viewFileBtn->setText(tr("View font directory"));
    m_viewFileBtn->setFixedSize(180, 36);
    m_viewFileBtn->setFocusPolicy(Qt::NoFocus);
    m_viewFileBtn->setVisible(false);

    m_closeBtn->setFixedSize(160, 36);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setVisible(false);
    m_closeBtn->setText(tr("Done"));

    // connect the signals to the slots function.
    connect(m_fontManager, &DFontManager::installFinished, this, &SingleFilePage::onInstallFinished);
    connect(m_fontManager, &DFontManager::uninstallFinished, this, &SingleFilePage::onUninstallFinished);
    connect(m_fontManager, &DFontManager::reinstallFinished, this, &SingleFilePage::onReinstallFinished);;;
    connect(m_fontManager, &DFontManager::reinstalling, this, &SingleFilePage::progressBarStart);
    connect(m_fontManager, &DFontManager::uninstalling, this, &SingleFilePage::progressBarStart);
    connect(m_installBtn, &QPushButton::clicked, this, &SingleFilePage::handleInstall);
    connect(m_uninstallBtn, &QPushButton::clicked, this, &SingleFilePage::handleRemove);
    connect(m_reinstallBtn, &QPushButton::clicked, this, &SingleFilePage::handleReinstall);
    connect(m_viewFileBtn, &QPushButton::clicked, this, &SingleFilePage::viewFilePath);
    connect(m_closeBtn, &QPushButton::clicked, this, &QApplication::quit);

    connect(m_fontManager, &DFontManager::installPositionChanged, this,
            [=] (const QString &path) {
                progressBarStart();
                m_filePath = path.trimmed();
            });
}

SingleFilePage::~SingleFilePage()
{
}

void SingleFilePage::updateInfo(DFontInfo *info)
{
    m_fontInfo = info;
    refreshPage();

    const QFontMetrics fm(m_copyrightLabel->font());
    m_nameLabel->setText(m_fontInfo->familyName);
    m_styleLabel->setText(m_fontInfo->styleName);
    m_typeLabel->setText(m_fontInfo->type);
    m_versionLabel->setText(fm.elidedText(m_fontInfo->version, Qt::ElideRight, 300));
    const QSize boundingSize = QSize(m_descriptionLabel->width(), m_descriptionLabel->maximumHeight());
    m_copyrightLabel->setText(holdTextInRect(m_copyrightLabel->font(), m_fontInfo->copyright, boundingSize));
    m_descriptionLabel->setText(holdTextInRect(m_descriptionLabel->font(), m_fontInfo->description, boundingSize));
}

void SingleFilePage::refreshPage()
{
    const bool isInstalled = m_fontInfo->isInstalled;
    const bool isSameVersion = m_fontInfo->sysVersion == m_fontInfo->version;

    if (isInstalled) {
        m_installBtn->setVisible(false);
        m_uninstallBtn->setVisible(true);
        m_reinstallBtn->setVisible(true);
        m_viewFileBtn->setVisible(false);
        m_closeBtn->setVisible(false);

        if (isSameVersion) {
            m_tipsLabel->setStyleSheet("QLabel { color: #FF5A5A; font-size: 14px; }");
            m_tipsLabel->setText(tr("Same version installed"));
        } else {
            m_tipsLabel->setStyleSheet("QLabel { color: #FF5A5A; font-size: 14px; }");
            m_tipsLabel->setText(QString(tr("Other version installed: %1")).arg(m_fontInfo->sysVersion));
        }
    } else {
        m_tipsLabel->setText("");
        m_installBtn->setVisible(true);
        m_uninstallBtn->setVisible(false);
        m_reinstallBtn->setVisible(false);
        m_viewFileBtn->setVisible(false);
        m_closeBtn->setVisible(false);
    }
}

void SingleFilePage::progressBarStart()
{
    m_bottomLayout->setCurrentIndex(1);
    m_tipsLabel->setText("");
    m_spinner->start();
}

void SingleFilePage::handleInstall()
{
    qDebug() << "Install file: " << m_fontInfo->filePath;

    m_fontManager->setType(DFontManager::Install);
    m_fontManager->setInstallFileList(QStringList() << m_fontInfo->filePath);
    m_fontManager->start();
}

void SingleFilePage::handleRemove()
{
    m_fontInfoManager->refreshList();
    const QString target = m_fontInfoManager->getInstalledFontPath(m_fontInfo);

    m_fontManager->setType(DFontManager::UnInstall);
    m_fontManager->setUnInstallFile(target);
    m_fontManager->start();
}

void SingleFilePage::handleReinstall()
{
    const QString sysPath = m_fontInfoManager->getInstalledFontPath(m_fontInfo);
    m_filePath = sysPath;

    m_fontManager->setType(DFontManager::ReInstall);
    m_fontManager->setReInstallFile(m_fontInfo->filePath, sysPath);
    m_fontManager->start();
}

void SingleFilePage::onInstallFinished()
{
    m_fontInfo->isInstalled = true;

    m_tipsLabel->setStyleSheet("QLabel { color: #47790c; font-size: 14px; }");
    m_tipsLabel->setText(tr("Installed successfully"));
    m_installBtn->setVisible(false);
    m_uninstallBtn->setVisible(false);
    m_reinstallBtn->setVisible(false);
    m_viewFileBtn->setVisible(true);
    m_closeBtn->setVisible(false);

    m_spinner->stop();
    m_bottomLayout->setCurrentIndex(0);
}

void SingleFilePage::onUninstallFinished()
{
    m_fontInfo->isInstalled = false;

    m_tipsLabel->setStyleSheet("QLabel { color: #47790c; font-size: 14px; }");
    m_tipsLabel->setText(tr("Removed successfully"));
    m_installBtn->setVisible(false);
    m_uninstallBtn->setVisible(false);
    m_reinstallBtn->setVisible(false);
    m_viewFileBtn->setVisible(false);
    m_closeBtn->setVisible(true);

    m_spinner->stop();
    m_bottomLayout->setCurrentIndex(0);
}

void SingleFilePage::onReinstallFinished()
{
    m_fontInfo->isInstalled = true;
    m_tipsLabel->setStyleSheet("QLabel { color: #47790c; font-size: 14px; }");
    m_tipsLabel->setText(tr("Installed successfully"));
    m_installBtn->setVisible(false);
    m_uninstallBtn->setVisible(false);
    m_reinstallBtn->setVisible(false);
    m_viewFileBtn->setVisible(true);
    m_closeBtn->setVisible(false);

    m_spinner->stop();
    m_bottomLayout->setCurrentIndex(0);
}

void SingleFilePage::viewFilePath()
{
    QUrl url = QUrl::fromLocalFile(QFileInfo(m_filePath).dir().absolutePath());
    QUrlQuery query;
    query.addQueryItem("selectUrl", QUrl::fromLocalFile(m_filePath).toString());
    url.setQuery(query);

    QProcess::startDetached("dde-file-manager", QStringList(url.toString()));}
