#pragma once

#include <DWidget>
#include <QLabel>

class DFontSpinner;
class DFontSpinnerWidget : public Dtk::Widget::DWidget
{
public:
    enum SpinnerStyles {
        StartupLoad,
        Load,
        Delete,
        NoLabel,
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
    DFontSpinner *m_spinner = nullptr;
    QLabel *m_label = nullptr;
};
