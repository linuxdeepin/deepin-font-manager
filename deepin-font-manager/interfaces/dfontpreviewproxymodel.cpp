#include "dfontpreviewproxymodel.h"
#include "dfontpreviewitemdef.h"
#include "views/dsplitlistwidget.h"
#include "dfontpreviewlistdatathread.h"

#include <DLog>

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

bool DFontPreviewProxyModel::getEditStatus() const
{
    return m_editStatus;
}

void DFontPreviewProxyModel::invalidate()
{
    qDebug() << __FUNCTION__;
    QSortFilterProxyModel::invalidate();
}

void DFontPreviewProxyModel::setEditStatus(bool editStatus)
{
    m_editStatus = editStatus;
}

bool DFontPreviewProxyModel::isFontNameContainsPattern(QString fontName) const
{
    if (m_fontNamePattern.length() > 0) {
        return fontName.contains(m_fontNamePattern, Qt::CaseInsensitive);
    }

    return true;
}

bool DFontPreviewProxyModel::isCustomFilterAcceptsRow(const QModelIndex &modelIndex) const
{
    QVariant varModel = sourceModel()->data(modelIndex, Qt::DisplayRole);
    if (varModel.isValid() == false) {
        return false;
    }

    //zhangya 20200313  fix varModel is not DFontPreviewItemData crash
    if (varModel.canConvert<DFontPreviewItemData>() == false) {
        return false;
    }

    DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

    const QString &fontName = itemData.strFontName;

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
        QString fontFilePath = itemData.fontInfo.filePath;
        if (fontFilePath.startsWith("/usr/share/fonts/") &&
                !fontFilePath.contains("/.local/share/fonts/") &&
                isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //只显示用户字体
    case DSplitListWidget::UserFont: {
        QString fontFilePath = itemData.fontInfo.filePath;
        if (fontFilePath.contains("/.local/share/fonts/") &&
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
        if (itemData.isChineseFont && isFontNameContainsPattern(fontName)) {
            return true;
        }
    } break;
    //等宽字体
    case DSplitListWidget::EqualWidthFont: {
        if (itemData.isMonoSpace && isFontNameContainsPattern(fontName)) {
            return true;
        }

    } break;
    default:
        break;
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

    if (modelIndex.isValid() == false) {
        return false;
    }

    if (isCustomFilterAcceptsRow(modelIndex)) {
        return true;
    } else {
        return false;
    }
}

int DFontPreviewProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    int filterRowCount = QSortFilterProxyModel::rowCount();
    //qDebug() << "filterRowCount" << filterRowCount;

    unsigned int bShow = 0;
    if (0 == filterRowCount) {
        if (getEditStatus()) {
            bShow = 2;
        } else {
            bShow = 1;
        }
    } else {
        bShow = 0;
    }

    emit this->onFilterFinishRowCountChangedInt(bShow);

    return filterRowCount;
}
