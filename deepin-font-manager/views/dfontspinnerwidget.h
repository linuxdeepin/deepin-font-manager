#ifndef DFONTSPINNERWIDGET_H
#define DFONTSPINNERWIDGET_H

#include <DSpinner>
#include <DWidget>
#include <QLabel>

class DFontSpinnerWidget : public Dtk::Widget::DWidget
{
public:
    explicit DFontSpinnerWidget(QWidget *parent = nullptr);
    void initUI();

    void spinnerStart();
    void spinnerStop();
    ~DFontSpinnerWidget();

private:
    Dtk::Widget::DSpinner *m_spinner = nullptr;
    QLabel *m_label = nullptr;
};

#endif // DFONTSPINNERWIDGET_H
