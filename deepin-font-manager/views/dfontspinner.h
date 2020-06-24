#pragma once

#include <QWidget>

class DFontSpinnerPrivate;
class DFontSpinner : public QWidget
{
    Q_OBJECT
public:
    explicit DFontSpinner(QWidget *parent = nullptr);

public Q_SLOTS:
    void start();
    void stop();
    void setBackgroundColor(QColor color);

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *e) override;

private:
    DFontSpinnerPrivate *const d_ptr {nullptr};
    Q_DECLARE_PRIVATE(DFontSpinner)
};
