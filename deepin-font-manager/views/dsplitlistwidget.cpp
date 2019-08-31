#include "dsplitlistwidget.h"

#include <DHorizontalLine>
#include <DLog>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

//用于去除选中项的边框
void DNoFocusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    if (itemOption.state & QStyle::State_HasFocus) {
        itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter, itemOption, index);
}

DSplitListWidget::DSplitListWidget(QWidget *parent)
    : QListWidget(parent)
{
    //去除选中项的边框
    this->setItemDelegate(new DNoFocusDelegate());

    initListData();
    initConnections();
}

DSplitListWidget::~DSplitListWidget() {}

QWidget *DSplitListWidget::initSplitWidget(int widgetWidth, int widgetHeight)
{
    QWidget *splitWidget = new QWidget(this);

    QVBoxLayout *vBoxLayout = new QVBoxLayout(splitWidget);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->setMargin(0);
    vBoxLayout->setSpacing(0);
    splitWidget->setLayout(vBoxLayout);

    DHorizontalLine *splitHorizontal = new DHorizontalLine(splitWidget);
    int splitHeight = splitHorizontal->height();

    QFrame *frameTop = new QFrame(splitWidget);
    frameTop->setFixedSize(widgetWidth, (widgetHeight - splitHeight) / 2);
    vBoxLayout->addWidget(frameTop);

    vBoxLayout->addWidget(splitHorizontal);

    QFrame *frameBottom = new QFrame(splitWidget);
    frameBottom->setFixedSize(widgetWidth, (widgetHeight - splitHeight) / 2);
    vBoxLayout->addWidget(frameBottom);

    return splitWidget;
}

void DSplitListWidget::initListData()
{
#warning need internationalization
    m_titleStringList << QString("所有字体") << QString("系统字体") << QString("用户字体")
                      << QString("我的收藏") << QString("已激活") << QString("中文")
                      << QString("等宽");

    for (int i = 0; i < m_titleStringList.size(); i++) {
        QString titleString = m_titleStringList.at(i);
        QListWidgetItem *item = new QListWidgetItem(titleString);
        item->setSizeHint(QSize(120, 40));
        item->setData(Qt::UserRole, QVariant::fromValue(i));

        this->addItem(item);

        if (0 == i) {
            item->setSelected(true);
        }
    }

    this->insertSplitItem(5);
}

void DSplitListWidget::initConnections()
{
    QObject::connect(this, SIGNAL(itemClicked(QListWidgetItem *)), this,
                     SLOT(onListWidgetItemClicked(QListWidgetItem *)));
}

void DSplitListWidget::insertSplitItem(int row)
{
    int splitCount = 0;
    QSize itemSize;
    for (int i = 0; i < this->count(); i++) {
        QListWidgetItem *item = this->item(i);
        QWidget *widget = this->itemWidget(item);
        if (nullptr != widget) {
            ++splitCount;
        } else {
            itemSize = item->sizeHint();
        }
    }
    int insertIndex = splitCount + row;

    QListWidgetItem *itemSperator = new QListWidgetItem();
    itemSperator->setSizeHint(itemSize);
    itemSperator->setData(Qt::UserRole, QVariant(-1));
    itemSperator->setFlags(itemSperator->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);

    QWidget *splitWidget = initSplitWidget(itemSize.width(), itemSize.height());
    this->insertItem(insertIndex, itemSperator);
    this->setItemWidget(itemSperator, splitWidget);
}

void DSplitListWidget::onListWidgetItemClicked(QListWidgetItem *item)
{
    int rowIndex = this->row(item);
    emit onListWidgetItemClicked(rowIndex);
}
