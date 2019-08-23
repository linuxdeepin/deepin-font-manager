#include "dfontinfodialog.h"

#include <QTextBlock>
#include <QVBoxLayout>

DFontInfoDialog::DFontInfoDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnections();
}

void DFontInfoDialog::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));

    // ToDo:
    //    Need localize the string

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->setSpacing(0);

    m_mainFrame = new QFrame(this);
    m_mainFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Font logo
    m_fontLogo = new DLabel(this);
    m_fontLogo->setFixedSize(QSize(128, 128));
    m_fontLogo->setPixmap(QPixmap(":/images/font-info-logo.svg"));

    // Font file name
    m_fontFileName = new DLabel(this);
    m_fontFileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontFileName->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    m_fontFileName->setFixedHeight(18);
    m_fontFileName->setText("SourceHanSansSC-ExtraLight");

    /**************************Basic info panel****BEGIN*******************************/
    m_basicInfoFrame = new QFrame(this);
    m_basicInfoFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_basicInfoFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGridLayout *baseicInfoLayout = new QGridLayout(this);
    baseicInfoLayout->setAlignment(Qt::AlignTop /*| Qt::AlignVCenter*/);
    baseicInfoLayout->setContentsMargins(10, 5, 0, 0);
    baseicInfoLayout->setSpacing(8);

    DLabel *panelName = new DLabel(this);
    panelName->setFixedSize(QSize(60, 20));
    panelName->setText("基本信息");

    // Style Row
    DLabel *styleName = new DLabel(this);
    styleName->setFixedSize(QSize(60, 18));
    styleName->setText("样式");

    QHBoxLayout *styleLayout = new QHBoxLayout(this);
    styleLayout->setSpacing(0);

    m_fontSytleName = new DLabel(this);
    m_fontSytleName->setFixedHeight(18);
    m_fontSytleName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontSytleName->setText("Regular");
    styleLayout->addSpacing(4);
    styleLayout->addWidget(m_fontSytleName);

    // Type Row
    DLabel *typeName = new DLabel(this);
    typeName->setFixedSize(QSize(60, 18));
    typeName->setText("类型");

    QHBoxLayout *typeLayout = new QHBoxLayout(this);
    typeLayout->setSpacing(0);

    m_fontTypeName = new DLabel(this);
    m_fontTypeName->setFixedHeight(18);
    m_fontTypeName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontTypeName->setText("True Type");
    typeLayout->addSpacing(4);
    typeLayout->addWidget(m_fontTypeName);

    // Version row
    DLabel *versionName = new DLabel(this);
    versionName->setFixedSize(QSize(60, 18));
    versionName->setText("版本");

    m_fontVersion = new QTextEdit(this);
    m_fontVersion->setFixedHeight(72);
    m_fontVersion->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_fontVersion->setFrameShape(QFrame::Shape::NoFrame);
    m_fontVersion->setContentsMargins(0, 0, 0, 0);
    // ToDo:
    //    Need store in config file
    //    QFont ft;
    //    ft.setPixelSize(12);
    //    m_fontVersion->setFont(ft);
    m_fontVersion->setText(
        "Copyright 2014~2015 Adobe Syste-ms Incorporated (http://www.adob.com/), with Reserved "
        "Font Name cc Source.");

    // Description row
    DLabel *despName = new DLabel(this);
    despName->setFixedSize(QSize(60, 18));
    despName->setText("描述");

    /* Text in QTextEdit have about 4px left-margin,that can't
     * be removed, so add a QHBoxLayout in row tow and add 4px space
     * to align row two
     */
    QHBoxLayout *depsLayout = new QHBoxLayout(this);
    depsLayout->setSpacing(0);

    m_fontDescription = new DLabel(this);
    m_fontDescription->setFixedHeight(18);
    m_fontDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fontDescription->setText("未知");
    depsLayout->addSpacing(4);
    depsLayout->addWidget(m_fontDescription);

    baseicInfoLayout->addWidget(panelName, 0, 0);
    baseicInfoLayout->addWidget(styleName, 1, 0);
    baseicInfoLayout->addLayout(styleLayout, 1, 1);
    baseicInfoLayout->addWidget(typeName, 2, 0);
    baseicInfoLayout->addLayout(typeLayout, 2, 1);
    baseicInfoLayout->addWidget(versionName, 3, 0, Qt::AlignTop);
    baseicInfoLayout->addWidget(m_fontVersion, 3, 1);
    baseicInfoLayout->addWidget(despName, 4, 0);
    baseicInfoLayout->addLayout(depsLayout, 4, 1);

    m_basicInfoFrame->setLayout(baseicInfoLayout);
    /**************************Basic info panel****END*******************************/

    // Add childs to main layout
    mainLayout->addSpacing(58);
    mainLayout->addWidget(m_fontLogo, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(16);
    mainLayout->addWidget(m_fontFileName);
    mainLayout->addSpacing(45);
    mainLayout->addWidget(m_basicInfoFrame);

    m_mainFrame->setLayout(mainLayout);

#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_mainFrame->setStyleSheet("background: red");
    m_fontLogo->setStyleSheet("background: silver");
    m_fontFileName->setStyleSheet("background: silver");
    m_basicInfoFrame->setStyleSheet("background: yellow");
    panelName->setStyleSheet("background: blue");
    // versionName->setStyleSheet("background: green");
    // m_fontVersion->setStyleSheet("background: green");
#endif
}

void DFontInfoDialog::initConnections() {}

void DFontInfoDialog::resizeEvent(QResizeEvent *event)
{
    DDialog::resizeEvent(event);

    m_mainFrame->setFixedSize(event->size().width(), event->size().height());
}
