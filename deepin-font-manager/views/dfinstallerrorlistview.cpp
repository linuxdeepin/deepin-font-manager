#include "dfinstallerrorlistview.h"
#include "globaldef.h"
#include "utils.h"

#include <QPainter>
#include <QMouseEvent>

#include <DLog>
#include <DStyleHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DCheckBox>

#define FTM_ERROR_ITEM_FONTNAME_LEFT    39

//DFInstallErrorListDelegate

DFInstallErrorListDelegate::DFInstallErrorListDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

void DFInstallErrorListDelegate::drawCheckBox(QPainter *painter, DFInstallErrorItemModel itemModel, QRect bgRect) const
{
    QStyleOptionButton checkBoxOption;
    bool checked = itemModel.bChecked;
    checkBoxOption.state |= QStyle::State_Enabled;
    //根据值判断是否选中
    if (checked) {
        checkBoxOption.state |= QStyle::State_On;
    } else {
        checkBoxOption.state |= QStyle::State_Off;
    }

    int checkBoxSize = 20;
    DCheckBox checkBox;
    QRect checkboxRect = QRect(bgRect.left() + 10, bgRect.top() + 14 + 2, checkBoxSize - 4, checkBoxSize - 4);
    checkBoxOption.rect = checkboxRect;
    DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox,
                                         &checkBoxOption,
                                         painter,
                                         &checkBox);
}

void DFInstallErrorListDelegate::drawCheckBoxIcon(QPainter *painter, QRect bgRect) const
{

    int checkBoxSize = 20;
    QRect checkboxRect = QRect(bgRect.left() + 5, bgRect.top() + 10, checkBoxSize + 10, checkBoxSize + 10);
//    QRect checkboxRect = QRect(bgRect.left() + 0, bgRect.top(), checkBoxSize - 4, checkBoxSize - 4);
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        QImage checkBox("://checkbox_unchecked_light.svg");
        painter->drawImage(checkboxRect, checkBox);
    } else {
        QImage checkBox("://checkbox_unchecked_dark.svg");
        painter->drawImage(checkboxRect, checkBox);
    }
}

void DFInstallErrorListDelegate::drawFontName(QPainter *painter, const QStyleOptionViewItem &option,
                                              DFInstallErrorItemModel itemModel, QRect bgRect, bool bSelectable) const
{
    QFont nameFont = painter->font();
    nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
    painter->setFont(nameFont);

    QString strFontFileName = itemModel.strFontFileName;
    QString strStatus = itemModel.strFontInstallStatus;


    int checkBoxSize = 20;
    QRect checkboxRect = QRect(bgRect.left() + 10, bgRect.top() + 14 + 2, checkBoxSize - 4, checkBoxSize - 4);
    int statusLabelMaxWidth = 160;
    int fontNameLeft = FTM_ERROR_ITEM_FONTNAME_LEFT;

    QFontMetrics fontMetric(nameFont);
    int m_StatusWidth = fontMetric.width(strStatus);
    if (m_StatusWidth > statusLabelMaxWidth)
        m_StatusWidth = statusLabelMaxWidth;
    int m_NameWidth = fontMetric.width(strFontFileName);

    QRect fontFileNameRect = QRect(bgRect.left() + fontNameLeft,
                                   checkboxRect.top() - 5,
                                   bgRect.width() - fontNameLeft - m_StatusWidth,
                                   checkboxRect.height() + 15);




//    QString elidedFontFileNameText = fontMetric.elidedText(strFontFileName,
//                                                           Qt::ElideRight,
//                                                           fontFileNameRect.width(),
//                                                           Qt::TextShowMnemonic);

    //Automatically truncates and adds ellipsis based on the font width /*UT000539*/
    //QString elidedFontFileNameText = fontMetric.elidedText(strFontFileName, Qt::ElideRight, 235);
    //ut000442 Optimize the adaptive effect here 20200421
    QString elidedFontFileNameText;

    if (m_NameWidth  + m_StatusWidth < 360) {
        elidedFontFileNameText = strFontFileName;
    } else {
        elidedFontFileNameText = lengthAutoFeed(painter, strFontFileName, m_StatusWidth);
    }


    if (option.state & QStyle::State_Selected) {
        QColor penColor = option.palette.color(DPalette::Text);
        if (bSelectable == false) {
            penColor.setAlphaF(0.4);
        }
        painter->setPen(QPen(penColor));
        painter->drawText(fontFileNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedFontFileNameText);
    } else {
        QColor penColor = option.palette.color(DPalette::Text);
        if (bSelectable == false) {
            penColor.setAlphaF(0.4);
        }
        painter->setPen(QPen(penColor));
        painter->drawText(fontFileNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedFontFileNameText);
    }
}

void DFInstallErrorListDelegate::drawFontStyle(QPainter *painter, const QStyleOptionViewItem &option,
                                               DFInstallErrorItemModel itemModel, QRect bgRect, bool bSelectable) const
{
    Q_UNUSED(option);
    Q_UNUSED(bSelectable);
    int statusLabelMaxWidth = 160;
    QColor penColor;
    QRect installStatusRect = QRect(bgRect.left() + (bgRect.width() - statusLabelMaxWidth) - 10,
                                    bgRect.top(),
                                    statusLabelMaxWidth,
                                    bgRect.height());
    QString strStatus = itemModel.strFontInstallStatus;


    QFont installStatusFont = painter->font();
    installStatusFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T9));
    painter->setFont(installStatusFont);


    QFont nameFont = painter->font();
    nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T9));
    painter->setFont(nameFont);

    QFontMetrics fontMetric(nameFont);
    QFontMetrics stateFontMetric(installStatusFont);
    QString elidedStatusText = fontMetric.elidedText(strStatus,
                                                     Qt::ElideRight,
                                                     installStatusRect.width(),
                                                     Qt::TextShowMnemonic);

//    DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
//    DStyleHelper styleHelper;
//    QColor penColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::TextWarning);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        penColor = QColor("#FF6D6D");
    } else {
        penColor = QColor("#FF4E4E");
    }


    painter->setPen(QPen(penColor));
    painter->drawText(installStatusRect, Qt::AlignRight | Qt::AlignVCenter, elidedStatusText);
    QFont::cleanup();
}

void DFInstallErrorListDelegate::drawSelectStatus(QPainter *painter, const QStyleOptionViewItem &option, QRect bgRect) const
{
    QPainterPath path;
    const int radius = 8;

    path.moveTo(bgRect.bottomRight() - QPoint(0, radius));
    path.lineTo(bgRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(bgRect.topRight() - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(bgRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(bgRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(bgRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(bgRect.bottomLeft() - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(bgRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(bgRect.bottomRight() - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);

    if (option.state & QStyle::State_Selected) {
//        DPalette pa = DApplicationHelper::instance()->palette(m_parentView);//dtk库接口不稳定，更换palette获取方式
        DPalette pa = DApplicationHelper::instance()->applicationPalette();
        DStyleHelper styleHelper;
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
        painter->fillPath(path, QBrush(fillColor));

    }
}

QString DFInstallErrorListDelegate::lengthAutoFeed(QPainter *painter, QString sourceStr, int m_StatusWidth) const
{
    //ut000442 listview中字体名长度自适应处理

    QFont nameFont = painter->font();
    QFontMetrics fontMetric(nameFont);

    QString m_TargetStr = sourceStr.left(1);
    QString m_Suffix("...");
    m_Suffix.append(sourceStr.right(5));
    m_TargetStr.append(m_Suffix);

    int m_TargetStrWidth = fontMetric.width(m_TargetStr);

    int m_index = 1;

    while (m_TargetStrWidth + m_StatusWidth < 350) {

//      每次插入一个字符，直到长度超过最大范围
        m_TargetStr.insert(m_index, sourceStr.at(m_index));
        m_TargetStrWidth = fontMetric.width(m_TargetStr);
        m_index++;
    }

    QFont::cleanup();
//    m_TargetStr.append(m_Suffix);
//    qDebug() << m_TargetStr;
    return m_TargetStr;

}



//用于去除选中项的边框
void DFInstallErrorListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    if (index.isValid()) {

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varErrorItem = index.data(Qt::DisplayRole);
        DFInstallErrorItemModel itemModel = varErrorItem.value<DFInstallErrorItemModel>();
        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        QRect rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width());
        rect.setHeight(option.rect.height());

        QRect bgRect = QRect(rect.left() + 10, rect.top(), rect.width() - 18, rect.height());

        if (itemModel.bSelectable) {
            drawSelectStatus(painter, option, bgRect);
            drawCheckBox(painter, itemModel, bgRect);
            drawFontName(painter, option, itemModel, bgRect);
            drawFontStyle(painter, option, itemModel, bgRect);
        } else {
            drawCheckBoxIcon(painter, bgRect);
            drawFontName(painter, option, itemModel, bgRect, false);
            drawFontStyle(painter, option, itemModel, bgRect, false);
        }
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
    QFont::cleanup();
}

QSize DFInstallErrorListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    Q_UNUSED(index)

    return QSize(option.rect.width(), 48);
}


//DFInstallErrorListView
DFInstallErrorListView::DFInstallErrorListView(const QList<DFInstallErrorItemModel> &installErrorFontModelList,
                                               QWidget *parent)
    : DListView(parent)
    , m_installErrorFontModelList(installErrorFontModelList)
{
    QWidget *topSpaceWidget = new QWidget;
    topSpaceWidget->setFixedSize(this->width(), 70 - FTM_TITLE_FIXED_HEIGHT);
    this->addHeaderWidget(topSpaceWidget);

    setAutoScroll(true);
    setMouseTracking(true);

    initErrorListData();
    initDelegate();
}

DFInstallErrorListView::~DFInstallErrorListView()
{
    m_errorListSourceModel->clear();
    m_installErrorFontModelList.clear();
}

void DFInstallErrorListView::initErrorListData()
{
    m_errorListSourceModel = new QStandardItemModel(this);
    DFontInfo fontInfo;
    for (int i = 0; i < m_installErrorFontModelList.size(); i++) {

        QStandardItem *item = new QStandardItem;
        DFInstallErrorItemModel itemModel = m_installErrorFontModelList.at(i);
        item->setData(QVariant::fromValue(itemModel), Qt::DisplayRole);
        m_errorListSourceModel->appendRow(item);

        fontInfo = m_fontInfoManager->getFontInfo(itemModel.strFontFilePath);
        m_errorFontlist.append(fontInfo.familyName + fontInfo.styleName);
    }

    this->setModel(m_errorListSourceModel);

    //设置默认选中第一行
    QModelIndex firstRowModelIndex = m_errorListSourceModel->index(0, 0);

    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_errorListSourceModel->data(firstRowModelIndex));

    if (itemModel.bSelectable) {
        selectionModel()->select(firstRowModelIndex, QItemSelectionModel::Select);
    }
}

void DFInstallErrorListView::initDelegate()
{
    m_errorListItemDelegate = new DFInstallErrorListDelegate(this);
    this->setItemDelegate(m_errorListItemDelegate);
}

void DFInstallErrorListView::addErrorListData(QList<DFInstallErrorItemModel> installErrorFontModelList)
{
    m_errorListSourceModel->clear();
    this->setModel(m_errorListSourceModel);

    QList<DFInstallErrorItemModel> m_newErrorFontModel;
    DFontInfo fontInfo;

    foreach (auto it, installErrorFontModelList) {
        fontInfo = m_fontInfoManager->getFontInfo(it.strFontFilePath);
        QString str = fontInfo.familyName + fontInfo.styleName;
        if (!m_errorFontlist.contains(fontInfo.familyName + fontInfo.styleName)) {
            m_newErrorFontModel.append(it);
            m_errorFontlist.append(fontInfo.familyName + fontInfo.styleName);
        }
    }
    m_installErrorFontModelList.append(m_newErrorFontModel);

//    QSet<QVariant> m_installErrorFontModelSet;

//    foreach (auto it, m_installErrorFontModelList) {
//        m_installErrorFontModelSet.insert(QVariant::fromValue(it));
//    }

//    qDebug() << m_installErrorFontModelSet.count() << endl;


    for (int i = 0; i < m_installErrorFontModelList.size(); i++) {
        QStandardItem *item = new QStandardItem;
        DFInstallErrorItemModel itemModel = m_installErrorFontModelList.at(i);
        item->setData(QVariant::fromValue(itemModel), Qt::DisplayRole);
        m_errorListSourceModel->appendRow(item);
    }

    this->setModel(m_errorListSourceModel);
}


//字体验证框中的滚动
void DFInstallErrorListView::checkScrollToIndex(QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles, QStringList &errorFileList)
{
    if (addHalfInstalledFiles.count() > 0) {
        scrollToIndex(addHalfInstalledFiles.first());
    } else if (addHalfInstalledFiles.count() == 0 && oldHalfInstalledFiles.count() != 0) {
        scrollToIndex(oldHalfInstalledFiles.first());
    } else if (addHalfInstalledFiles.count() == 0  && oldHalfInstalledFiles.count() == 0 && errorFileList.count() > 0) {
        scrollToIndex(errorFileList.first());
    } else {
        QModelIndex modelIndex = m_errorListSourceModel->index(beforeSelectRow, 0);
        scrollTo(modelIndex);
    }
}

void DFInstallErrorListView::scrollToIndex(QString &filePath)
{
    DFontInfo fontinfo = m_fontInfoManager->getFontInfo(filePath);
    QString file;
    if (!fontinfo.psname.compare("")) {
        file = fontinfo.familyName + fontinfo.styleName;
    } else {
        file = fontinfo.psname + fontinfo.styleName;
    }

    for (int i = 0; i < m_errorListSourceModel->rowCount(); i++) {
        QModelIndex modelIndex = m_errorListSourceModel->index(i, 0);
        QVariant varModel = m_errorListSourceModel->data(modelIndex, Qt::DisplayRole);
        DFInstallErrorItemModel itemData = varModel.value<DFInstallErrorItemModel>();
//            qDebug() << itemData.strFontFilePath << "!!!!!!!!!!!!" << endl;
        DFontInfo info =  m_fontInfoManager->getFontInfo(itemData.strFontFilePath);
        QString fileName;
        if (!info.psname.compare("")) {
            fileName = info.familyName + info.styleName;
        } else {
            fileName = info.psname + info.styleName;
        }
        if (file == fileName) {
            scrollTo(modelIndex);
        }
    }
}

//设置选中效果
void DFInstallErrorListView::setSelectStatus(QStringList &HalfInstalledFiles)
{
    QStringList str;
    foreach (auto it, HalfInstalledFiles) {
        DFontInfo fontinfo = m_fontInfoManager->getFontInfo(it);
        QString file;
        if (!fontinfo.psname.compare("")) {
            file = fontinfo.familyName + fontinfo.styleName;
        } else {
            file = fontinfo.psname + fontinfo.styleName;
        }
        str << file;
    }
    for (int i = 0; i < m_errorListSourceModel->rowCount(); i++) {
        QModelIndex modelIndex = m_errorListSourceModel->index(i, 0);
        QVariant varModel = m_errorListSourceModel->data(modelIndex, Qt::DisplayRole);
        DFInstallErrorItemModel itemData = varModel.value<DFInstallErrorItemModel>();
        DFontInfo info =  m_fontInfoManager->getFontInfo(itemData.strFontFilePath);
        QString fileName;
        if (!info.psname.compare("")) {
            fileName = info.familyName + info.styleName;
        } else {
            fileName = info.psname + info.styleName;
        }
        if (str.contains(fileName)) {
            if (!itemData.bChecked) {
                itemData.bChecked = true;
                updateErrorFontModelList(i, itemData);
                m_errorListSourceModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }
            if (itemData.bSelectable) {
                selectionModel()->select(modelIndex, QItemSelectionModel::Select);
            }
        }
    }
}

void DFInstallErrorListView::updateErrorFontModelList(int index, DFInstallErrorItemModel m_currentItemModel)
{
    m_installErrorFontModelList.replace(index, m_currentItemModel);
}

QStandardItemModel *DFInstallErrorListView::getErrorListSourceModel()
{
    return m_errorListSourceModel;
}

void DFInstallErrorListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);
}

void DFInstallErrorListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);

    QPoint selectionPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(selectionPoint);

    if (m_bLeftMouse) {
        DFInstallErrorItemModel itemModel =
            qvariant_cast<DFInstallErrorItemModel>(m_errorListSourceModel->data(modelIndex));
        if (!itemModel.bSelectable) {
            return;
        }

        if (selectionPoint.x() < FTM_ERROR_ITEM_FONTNAME_LEFT) {
            emit onClickErrorListItem(modelIndex);
        }
    }
}
