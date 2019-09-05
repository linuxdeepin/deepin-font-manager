#include "dfontpreviewproxymodel.h"
#include "dfontpreviewitemdef.h"
#include "views/dsplitlistwidget.h"

#include <DLog>

#include <QStandardItemModel>

DFontPreviewProxyModel::DFontPreviewProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_filterGroup(DSplitListWidget::AllFont)
    , m_useSystemFilter(true)
    , m_fontNamePattern("")
{
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    m_chineFontPathList = fontInfoManager->getAllChineseFontPath();
    m_monoSpaceFontPathList = fontInfoManager->getAllMonoSpaceFontPath();
//    qDebug() << m_chineFontPathList;
//    qDebug() << m_monoSpaceFontPathList;
}

void DFontPreviewProxyModel::setFilterGroup(int filterGroup)
{
    m_useSystemFilter = false;
    m_filterGroup = filterGroup;

    invalidateFilter();
}

void DFontPreviewProxyModel::setUseSystemFilter(bool useSystemFilter)
{
    m_useSystemFilter = useSystemFilter;

    invalidateFilter();
}

void DFontPreviewProxyModel::setFilterFontNamePattern(const QString &pattern)
{
    m_useSystemFilter = false;
    m_fontNamePattern = pattern;

    invalidateFilter();
}

bool DFontPreviewProxyModel::isFontNameContainsPattern(QString fontName) const
{
    if (m_fontNamePattern.length() > 0) {
        return fontName.contains(m_fontNamePattern, Qt::CaseInsensitive);
    }

    return true;
}

bool DFontPreviewProxyModel::isChineseFont(QString fontFilePath) const
{
    if (m_chineFontPathList.contains(fontFilePath)) {
        return true;
    }

    return false;
}

bool DFontPreviewProxyModel::isMonoSpaceFont(QString fontFilePath) const
{
    if (m_monoSpaceFontPathList.contains(fontFilePath)) {
        return true;
    }

    return false;
}

bool DFontPreviewProxyModel::isCustomFilterAcceptsRow(const QModelIndex &modelIndex) const
{
    QVariant varModel = sourceModel()->data(modelIndex, Qt::DisplayRole);
    DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

    QString fontName = itemData.strFontName;

    switch (m_filterGroup) {
    //显示所有字体
    case DSplitListWidget::AllFont: {
        if (m_fontNamePattern.length() > 0) {
            return isFontNameContainsPattern(fontName);
        }
        return true;
    }
    //只显示系统字体
    case DSplitListWidget::SysFont: {
        QString fontFilePath = itemData.pFontInfo->filePath;
        if (fontFilePath.startsWith("/usr/share/fonts/") &&
            !fontFilePath.contains("deepin-font-install") &&
            isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //只显示用户字体
    case DSplitListWidget::UserFont: {
        QString fontFilePath = itemData.pFontInfo->filePath;
        if (fontFilePath.contains("deepin-font-install") &&
            isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //只显示收藏字体
    case DSplitListWidget::CollectFont: {
        if (itemData.isCollected && isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //已激活(启用)字体
    case DSplitListWidget::ActiveFont: {
        if (itemData.isEnabled && isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //中文字体
    case DSplitListWidget::ChineseFont: {
        QString fontFilePath = itemData.pFontInfo->filePath;
        if (isChineseFont(fontFilePath) && isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //等宽字体
    case DSplitListWidget::EqualWidthFont: {
        QString fontFilePath = itemData.pFontInfo->filePath;
        if (isMonoSpaceFont(fontFilePath) && isFontNameContainsPattern(fontName)) {
            return true;
        }

    } break;
    }

    return false;
}

bool DFontPreviewProxyModel::filterAcceptsRow(int source_row,
                                              const QModelIndex &source_parent) const
{
    if (m_useSystemFilter) {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    QModelIndex modelIndex = sourceModel()->index(source_row, 0, source_parent);

    if (isCustomFilterAcceptsRow(modelIndex)) {
        return true;
    } else {
        return false;
    }
}
