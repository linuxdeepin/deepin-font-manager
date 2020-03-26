#include "ddlabel.h"
Dtk::Widget::DDLabel::DDLabel(QWidget *parent, Qt::WindowFlags f) : DLabel(parent, f)
{

}

void Dtk::Widget::DDLabel::Settext(const QString &text,const int &width)
{
        QFontMetrics elideFont(this->font());
        str = text;
        DLabel::setText(elideFont.elidedText(str, Qt::ElideRight, width));
}



void Dtk::Widget::DDLabel::paintEvent(QPaintEvent *event)
{
        DLabel::paintEvent(event);
        QFontMetrics elideFont(this->font());
        this->setText(elideFont.elidedText(str, Qt::ElideRight, 140));

}
