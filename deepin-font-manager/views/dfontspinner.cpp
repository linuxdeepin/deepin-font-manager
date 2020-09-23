#include "dfontspinner.h"

#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QEvent>
#include <QDebug>

class DFontSpinnerPrivate : public QObject
{
public:
    explicit DFontSpinnerPrivate(DFontSpinner *parent);
    virtual ~DFontSpinnerPrivate();

    QList<QColor> createDefaultIndicatorColorList(QColor color);

    QTimer refreshTimer;

    double indicatorShadowOffset = 10;
    double currentDegree = 0.0;

    QList<QList<QColor>> indicatorColors;
    DFontSpinner *const q_ptr;
    Q_DECLARE_PUBLIC(DFontSpinner)
};

DFontSpinnerPrivate::DFontSpinnerPrivate(DFontSpinner *parent)
    : q_ptr(parent)
{

}

DFontSpinnerPrivate::~DFontSpinnerPrivate()
{
    refreshTimer.stop();
}

DFontSpinner::DFontSpinner(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new DFontSpinnerPrivate(this))
{
    d_ptr->refreshTimer.setInterval(30);

    connect(&d_ptr->refreshTimer, &QTimer::timeout,
    this, [ = ]() {
        d_ptr->currentDegree += 14;
        update();
    });
}

DFontSpinner::~DFontSpinner()
{
    delete d_ptr;
}

void DFontSpinner::start()
{
    d_ptr->currentDegree += 14;
    update();
    d_ptr->refreshTimer.start();
}

void DFontSpinner::stop()
{
    d_ptr->refreshTimer.stop();
}

void DFontSpinner::setBackgroundColor(QColor color)
{
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Background, color);
    setPalette(pal);
}

void DFontSpinner::paintEvent(QPaintEvent *)
{
    if (d_ptr->indicatorColors.isEmpty()) {
        for (int i = 0; i < 3; ++i)
            d_ptr->indicatorColors << d_ptr->createDefaultIndicatorColorList(palette().highlight().color());
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    auto degreeCurrent = d_ptr->currentDegree * 1.0;

    auto center = QRectF(rect()).center();
    auto radius = qMin(rect().width(), rect().height()) / 2.0;
    auto indicatorRadius = radius / 2 / 2 * 1.1;
    auto indicatorDegreeDelta = 360 / d_ptr->indicatorColors.count();

    for (int i = 0; i <  d_ptr->indicatorColors.count(); ++i) {
        auto colors = d_ptr->indicatorColors.value(i);
        for (int j = 0; j < colors.count(); ++j) {
            degreeCurrent = d_ptr->currentDegree - j * d_ptr->indicatorShadowOffset + indicatorDegreeDelta * i;
            auto x = (radius - indicatorRadius) * qCos(qDegreesToRadians(degreeCurrent));
            auto y = (radius - indicatorRadius) * qSin(qDegreesToRadians(degreeCurrent));

            x = center.x() + x;
            y = center.y() + y;
            auto tl = QPointF(x - 1 * indicatorRadius, y - 1 * indicatorRadius);
            QRectF rf(tl.x(), tl.y(), indicatorRadius * 2, indicatorRadius * 2);

            QPainterPath path;
            path.addEllipse(rf);

            painter.fillPath(path, colors.value(j));
        }
    }
}

void DFontSpinner::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange)
        d_ptr->indicatorColors.clear();

    QWidget::changeEvent(e);
}

QList<QColor> DFontSpinnerPrivate::createDefaultIndicatorColorList(QColor color)
{
    QList<QColor> colors;
    QList<int> opacitys;
    opacitys << 100 << 30 << 15 << 10 << 5 << 4 << 3 << 2 << 1;
    for (int i = 0; i < opacitys.count(); ++i) {
        color.setAlpha(255 * opacitys.value(i) / 100);
        colors << color;
    }
    return colors;
}
