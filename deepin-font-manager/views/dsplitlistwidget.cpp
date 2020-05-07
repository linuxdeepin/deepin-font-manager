#include "dsplitlistwidget.h"
#include "globaldef.h"
#include "utils.h"
#include <QPainter>
#include <QMouseEvent>

#include <DStyleHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DLog>
#include <DFontSizeManager>
#include <QToolTip>

#define FTM_SPLIT_TOP_SPACE_TAG "_space_"
#define FTM_SPLIT_TOP_SPLIT_TAG "_split_"
#define FTM_SPLIT_LINE_INDEX    5
#define TITLE_VISIBLE_WIDTH    96
#define IS_NEED_ELLIPSIS  30 //是否需要省略号

DNoFocusDelegate::DNoFocusDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{

}

//用于去除选中项的边框
void DNoFocusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varDisplay = index.data(Qt::DisplayRole);
        QString iniTitle = varDisplay.value<QString>();
        QFont nameFont = painter->font();
        QString strTitle = adjustLength(iniTitle, nameFont);


        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        if (strTitle.startsWith(FTM_SPLIT_TOP_SPACE_TAG)) {
            //用于ListView顶部空白
        } else if (strTitle.startsWith(FTM_SPLIT_TOP_SPLIT_TAG)) {

            QRect lineRect;
            lineRect.setX(option.rect.x() + 10);
            lineRect.setY(option.rect.y() + option.rect.height() - 2);
            lineRect.setWidth(option.rect.width() - 20);
            lineRect.setHeight(2);
            //绘制分割线
            DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->fillRect(lineRect, fillColor);
        } else {

            QRect rect;
            rect.setX(option.rect.x());
            rect.setY(option.rect.y());
            rect.setWidth(option.rect.width());
            rect.setHeight(option.rect.height());

            QRect backgroundRect = QRect(rect.left() + 10, rect.top(), rect.width() - 20, rect.height());

            QPainterPath path;
            const int radius = 8;

            path.moveTo(backgroundRect.bottomRight() - QPoint(0, radius));
            path.lineTo(backgroundRect.topRight() + QPoint(0, radius));
            path.arcTo(QRect(QPoint(backgroundRect.topRight() - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
            path.lineTo(backgroundRect.topLeft() + QPoint(radius, 0));
            path.arcTo(QRect(QPoint(backgroundRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
            path.lineTo(backgroundRect.bottomLeft() - QPoint(0, radius));
            path.arcTo(QRect(QPoint(backgroundRect.bottomLeft() - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
            path.lineTo(backgroundRect.bottomLeft() + QPoint(radius, 0));
            path.arcTo(QRect(QPoint(backgroundRect.bottomRight() - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);

            if (option.state & QStyle::State_Selected) {
                QColor fillColor = option.palette.color(cg, DPalette::Highlight);
                painter->setBrush(QBrush(fillColor));
                painter->fillPath(path, painter->brush());
            }

            //绘制标题
            /* bug#20266 UT000591 */ /*bug 21075 ut000442*/
            QRect fontNameRect;
            if (strTitle == "System") {
                fontNameRect = QRect(backgroundRect.left() + 10, backgroundRect.top() + 1, backgroundRect.width() - 20, backgroundRect.height() - 7);
            } else {
                fontNameRect = QRect(backgroundRect.left() + 10, backgroundRect.top() + 2, backgroundRect.width() - 20, backgroundRect.height() - 7);
            }



            nameFont.setWeight(QFont::Medium);
            nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
            painter->setFont(nameFont);

            if (option.state & QStyle::State_Selected) {
                painter->setPen(QPen(option.palette.color(DPalette::HighlightedText)));
                painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, strTitle);
            } else {
                painter->setPen(QPen(option.palette.color(DPalette::Text)));
                painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, strTitle);
            }
        }

        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize DNoFocusDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    Q_UNUSED(index)

    int rowIndex = index.row();
    if (FTM_SPLIT_LINE_INDEX == rowIndex) {
        return QSize(option.rect.width(), 24);
    } else {
        return QSize(option.rect.width(), 36);
    }
}
//adjust length/*UT000539*/
QString DNoFocusDelegate::adjustLength(QString &titleName, QFont &font) const
{
    QFontMetrics fontMetric(font);

    QString finalTitle = "";
    QString m_curTitle = "";

    int curWidth ;
    for (auto str : titleName) {
        m_curTitle += str;
        curWidth = fontMetric.width(m_curTitle);
        if (curWidth > TITLE_VISIBLE_WIDTH) {
            if (m_curTitle == titleName) {
                finalTitle = m_curTitle;
                break;
            } else {
                if (fontMetric.width("...") > IS_NEED_ELLIPSIS) {
                    finalTitle = m_curTitle;
                } else {

                    finalTitle =   m_curTitle.append("...");
                }
                break;
            }
        } else {
            finalTitle = titleName;
        }
    }
    return finalTitle;
}

DSplitListWidget::DSplitListWidget(QWidget *parent)
    : DListView(parent)
{
    //去除选中项的边框
    this->setItemDelegate(new DNoFocusDelegate(this));
    this->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    this->setAutoScroll(false);
    this->setMouseTracking(true);
    this->installEventFilter(this);
    initListData();
}

DSplitListWidget::~DSplitListWidget() {}

void DSplitListWidget::initListData()
{
    m_titleStringList << DApplication::translate("Category", "All Fonts")
                      << DApplication::translate("Category", "System")
                      << DApplication::translate("Category", "User")
                      << DApplication::translate("Category", "Favorites")
                      << DApplication::translate("Category", "Active")
                      << DApplication::translate("Category", "Chinese")
                      << DApplication::translate("Category", "Monospaced");
    for (int i = 0; i < m_titleStringList.size(); i++) {
        QString titleString = m_titleStringList.at(i);
        m_titleStringIndexMap.insert(titleString, i);
    }

    m_categoryItemModell = new QStandardItemModel(this);

    int iTitleIndex = 0;
    for (int i = 0; i < m_titleStringList.size() + 1; i++) {
        QStandardItem *item = new QStandardItem;
        if (FTM_SPLIT_LINE_INDEX == i) {
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
            item->setData(QVariant::fromValue(QString(FTM_SPLIT_TOP_SPLIT_TAG)), Qt::DisplayRole);
        } else {
            QString titleString = m_titleStringList.at(iTitleIndex++);
            item->setData(QVariant::fromValue(titleString), Qt::DisplayRole);
        }
        m_categoryItemModell->appendRow(item);
    }

    this->setModel(m_categoryItemModell);

    //设置默认选中
    QModelIndex index = m_categoryItemModell->index(AllFont, 0);
    selectionModel()->select(index, QItemSelectionModel::Select);
}

void DSplitListWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (current.row() < 0 || FTM_SPLIT_LINE_INDEX == current.row()) {
        return;
    }

    QStandardItem *item = m_categoryItemModell->item(current.row());
    QVariant varUserData = item->data(Qt::DisplayRole).value<QVariant>();
    qDebug() << "varUserData" << varUserData << endl;
    int realIndex = m_titleStringIndexMap.value(varUserData.toString());

    emit onListWidgetItemClicked(realIndex);
}

void DSplitListWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (QToolTip::isVisible()) {
        QToolTip::hideText();
    }
    return;
}

//hover for helper on leftListview /*UT000539*/
bool DNoFocusDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view
                                 , const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QToolTip::hideText();
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(Qt::DisplayRole).toString();
        qDebug() << __FUNCTION__ << "__now Hover is :__" << tooltip;
        if (tooltip.isEmpty() || tooltip == "_split_") {
            hideTooltipImmediately();
        } else {
            int tooltipsize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipsize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
//            QTimer::singleShot(1000, [ = ]() {
            QToolTip::showText(event->globalPos(), strtooltip, view);

//            QTimer::singleShot(500, []() {
//                QToolTip::hideText();
//            });
//            });
        }
        return false;
    }
    return DNoFocusDelegate::helpEvent(event, view, option, index);
}

//hide tooltip
void DNoFocusDelegate::hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}
