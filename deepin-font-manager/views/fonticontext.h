#pragma once

#include <QWidget>
#include <DSvgRenderer>

class FontIconText : public QWidget
{
    Q_OBJECT
public:
    explicit FontIconText(const QString picPath, QWidget *parent = nullptr);
    void setFontName(const QString &familyName, const QString &styleName);
    inline void setText(const QString &text) { if (!text.isEmpty()) m_text = text; }

protected:
    void paintEvent(QPaintEvent *event);
signals:

public slots:
private:
    QString m_picPath;
    DTK_GUI_NAMESPACE::DSvgRenderer *render;
    QFont m_font;
    QString m_text;
};
