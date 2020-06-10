#ifndef DFONTSPINNERWIDGET_H
#define DFONTSPINNERWIDGET_H

#include <DSpinner>
#include <DWidget>
#include <QLabel>

class DFontSpinnerWidget : public Dtk::Widget::DWidget
{
public:
    enum SpinnerStyles {
        Load,
        Delete,
    };


public:
    explicit DFontSpinnerWidget(QWidget *parent = nullptr,  SpinnerStyles styles = SpinnerStyles::Load);
    void initUI();

    void spinnerStart();
    void spinnerStop();
    ~DFontSpinnerWidget();

    void setStyles(SpinnerStyles styles);

private:

    SpinnerStyles m_Styles;
    Dtk::Widget::DSpinner *m_spinner = nullptr;
    QLabel *m_label = nullptr;
};

#endif // DFONTSPINNERWIDGET_H
