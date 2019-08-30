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
        return fontName.contains(m_fontNamePattern);
    }
    return true;
}

bool DFontPreviewProxyModel::isCustomFilterAcceptsRow(const QModelIndex &modelIndex) const
{
    QVariant varModel = sourceModel()->data(modelIndex, Qt::DisplayRole);
    //    qDebug() << "varModel" << varModel << endl;
    DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

    QString fontName = itemData.strFontName;
    //    qDebug() << "fontName" << fontName << endl;

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
#warning to do...
        //已激活字体
        case DSplitListWidget::ActiveFont: {
            if (/*todo... itemData is ActiveFont && */ isFontNameContainsPattern(fontName)) {
                return true;
            }
        } break;
        //中文字体
        case DSplitListWidget::ChineseFont: {
            if (/*todo... itemData is ChineseFont && */ isFontNameContainsPattern(fontName)) {
                return true;
            }
        } break;
        //等宽字体
        case DSplitListWidget::EqualWidthFont: {
            if (/*todo... itemData is EqualWidthFont && */ isFontNameContainsPattern(fontName)) {
                return true;
            }

        } break;
    }

    //    qDebug() << "end" << endl;

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
