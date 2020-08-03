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
    , m_parentView(qobject_cast<DSplitListWidget *>(parent))
{

}

//用于去除选中项的边框
void DNoFocusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setFont(DApplication::font());
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

            paintTabFocusBackground(painter, option, backgroundRect, cg);

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



void DNoFocusDelegate::paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &backgroundRect, const QPalette::ColorGroup cg) const
{
    //绘制左侧列表外部高亮区域的路径
    QPainterPath path;
    const int radius = 8;
    setPaintPath(backgroundRect, path, 0, 0, radius);

    //绘制左侧列表窗口色区域的路径
    QPainterPath path2;
    setPaintPath(backgroundRect, path2, 2, 2, 6);

    //绘制左侧列表内部高亮区域的路径
    QPainterPath path3;
    setPaintPath(backgroundRect, path3, 3, 3, 6);

    if (option.state & QStyle::State_Selected) {
        QColor fillColor = option.palette.color(cg, DPalette::Highlight);
        painter->setBrush(QBrush(fillColor));
        painter->fillPath(path, painter->brush());

        if (m_parentView->IsTabFocus()) {
            QColor fillColor = option.palette.color(cg, DPalette::Highlight);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, painter->brush());

            QColor fillColor2 = option.palette.color(cg, DPalette::Window);
            painter->setBrush(QBrush(fillColor2));
            painter->fillPath(path2, painter->brush());

            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path3, painter->brush());
        }
    }
}

/**
*  @brief  得到需要绘制区域的路径
*  @param[in]  bgRect listview一项的区域范围
*  @param[in]  path 需要绘制区域的路径
*  @param[in]  xDifference x方向需要变化的数值
*  @param[in]  yDifference y方向需要变化的数值
*  @param[in]  radius  圆弧半径
*/
void DNoFocusDelegate::setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const
{
    QPoint path_bottomRight(bgRect.bottomRight().x() - xDifference, bgRect.bottomRight().y() - yDifference);
    QPoint path_topRight(bgRect.topRight().x() - xDifference, bgRect.topRight().y() + yDifference);
    QPoint path_topLeft(bgRect.topLeft().x() + xDifference, bgRect.topLeft().y() + yDifference);
    QPoint path_bottomLeft(bgRect.bottomLeft().x() + xDifference, bgRect.bottomLeft().y() - yDifference);
    path.moveTo(path_bottomRight - QPoint(0, 10));
    path.lineTo(path_topRight + QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_topRight - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(path_topLeft + QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_topLeft), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(path_bottomLeft - QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_bottomLeft - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(path_bottomRight - QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_bottomRight - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);
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
    connect(m_signalManager, &SignalManager::setSpliteWidgetScrollEnable, this, &DSplitListWidget::setRefreshFinished);
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
    Q_UNUSED(previous);
    if (current.row() < 0 || FTM_SPLIT_LINE_INDEX == current.row()) {
        return;
    }

    QStandardItem *item = m_categoryItemModell->item(current.row());
    QVariant varUserData = item->data(Qt::DisplayRole).value<QVariant>();
    int realIndex = m_titleStringIndexMap.value(varUserData.toString());

    /*用于判断切换菜单后scrolltotop、nofocus...539*/
    Q_EMIT m_signalManager->changeView();
    emit onListWidgetItemClicked(realIndex);
}

void DSplitListWidget::setIsHalfWayFocus(bool IsHalfWayFocus)
{
    m_IsHalfWayFocus = IsHalfWayFocus;
}

bool DSplitListWidget::IsTabFocus() const
{
    return m_IsTabFocus;
}


//获取当前SplitListview的状态
FocusStatus &DSplitListWidget::getStatus()
{
    m_currentStatus.m_IsFirstFocus = this->m_IsFirstFocus;
    m_currentStatus.m_IsHalfWayFocus = this->m_IsHalfWayFocus;
    m_currentStatus.m_IsMouseClicked = this->m_IsMouseClicked;

    return m_currentStatus;
}

//设置SplitListView的状态
void DSplitListWidget::setCurrentStatus(const FocusStatus &currentStatus)
{
    m_currentStatus = currentStatus;
}

void DSplitListWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (QToolTip::isVisible()) {
        QToolTip::hideText();
    }
    return;
}

void DSplitListWidget::setRefreshFinished(bool isInstalling)
{
    if (isInstalling) {
        m_refreshFinished = false;
    } else {
        m_refreshFinished = true;
    }
}

void DSplitListWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_refreshFinished)
        return;
    int now = selectedIndexes().last().row();
    int next = now;
    if (event->delta() > 0) {
        if (now > 0) {
            if (now == 6) {
                next = now - 2;
            } else {
                next = now - 1;
            }
        } else {
            return;
        }
    } else {
        if (now < this->count() - 1) {
            if (now == 4) {
                next = now + 2;
            } else {
                next = now + 1;
            }
        } else {
            return;
        }
    }
    if (next == now) {
        return;
    } else {
        this->setModel(m_categoryItemModell);
        QStandardItem *item = m_categoryItemModell->item(next);
        QModelIndex modelIndex = m_categoryItemModell->indexFromItem(item);
        setCurrentIndex(modelIndex);
        Q_EMIT m_signalManager->changeView();
        if (next > 5) {
            emit  onListWidgetItemClicked(next - 1);
        } else {
            emit  onListWidgetItemClicked(next);
        }
    }
}

void DSplitListWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        if (currentIndex().row() == 0) {
            QModelIndex modelIndex = m_categoryItemModell->index(7, 0);
            setCurrentIndex(modelIndex);
        } else {
            DListView::keyPressEvent(event);
        }
    } else if (event->key() == Qt::Key_Down) {
        if (currentIndex().row() == 7) {
            QModelIndex modelIndex = m_categoryItemModell->index(0, 0);
            setCurrentIndex(modelIndex);
        } else {
            DListView::keyPressEvent(event);
        }
    } else {
        DListView::keyPressEvent(event);
    }
}

bool DSplitListWidget::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

//失去焦点的时候对这几个标志位进行复位
    if (event->type() == QEvent::FocusOut) {
        m_IsMouseClicked = false;
        m_IsTabFocus = false;
        m_IsLeftFocus = false;
        m_IsHalfWayFocus = false;
    }

//获取焦点时只要不是通过鼠标点击获取焦点以及不是打开软件自动设置的焦点以及不是其他过程中途设置的焦点，就是判断为通过tab获取到的焦点
    if (event->type() == QEvent::FocusIn) {
        if (!m_IsMouseClicked /*&& !m_IsLeftFocus*/ && !m_IsFirstFocus && !m_IsHalfWayFocus) {
            m_IsTabFocus = true;
        }
//        m_IsFristTimeFocus = false;
        m_IsFirstFocus = false;
    }
    return false;
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

void DSplitListWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);

    m_IsMouseClicked = true;
    if (Qt::RightButton == event->button() || Qt::MiddleButton == event->button()) {
        return;
    }
    //应该设置焦点，否则鼠标在其他区域release会导致缺失焦点。
    setFocus(Qt::MouseFocusReason);
//    if (!hasFocus() && m_IsTabFocus) {
//        m_IsTabFocus = false;
//    }

    if (modelIndex.row() == currentIndex().row())
        return;
    DListView::mousePressEvent(event);
}
