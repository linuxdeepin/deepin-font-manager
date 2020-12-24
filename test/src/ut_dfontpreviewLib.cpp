
#include "dfontpreview.h"
#include "dfontwidget.h"

#include <QApplication>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QTextStream>
#include <QFileInfo>
#include <QPainter>
#include <QScreen>
#include <QDebug>
#include <QPaintEvent>
#include <QFile>
#include <QDir>

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#define FTM_DEFAULT_PREVIEW_CN_TEXT QString("因理想而出生，为责任而成长")
#define FTM_DEFAULT_PREVIEW_EN_TEXT QString("Don't let your dreams be dreams")
#define FTM_DEFAULT_PREVIEW_DIGIT_TEXT QString("0123456789")

namespace {
class TestDFontPreview : public testing::Test
{

protected:
    void SetUp()
    {
        fp = new DFontPreview();
    }
    void TearDown()
    {

    }
    // Some expensive resource shared by all tests.
    DFontPreview *fp;
};

}

TEST_F(TestDFontPreview, checkSetFileUrl)
{
    //传入错误参数
    fp->setFileUrl("file:///usr/share/fonts/truetype/liberation/LiberationSans-Italic.ttf");

    //传入正常参数
    fp->setFileUrl("/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf");
    delete  fp;
//    qDebug() << sampleString << "++++++++++++++++++" << styleName << endl;

}

TEST_F(TestDFontPreview, checkPaintEvent)
{

    QRect r;
    QString filepath = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();

    qDebug() << fp->fontDatabase.addApplicationFontFromData(fileContent);

    fp->setFileUrl(filepath);
    QPaintEvent *p = new QPaintEvent(r);
    fp->m_error = false;
    //不要滚动条
    fp->paintEvent(p);

    //要滚动条
    fp->currentMaxWidth = 1500;
    fp->paintEvent(p);

    delete  fp;
}

TEST_F(TestDFontPreview, checkGetLanguageSampleString)
{
    fp->m_contents.clear();
    fp->m_contents.insert("es", "first");
    //不存在的语言
    EXPECT_TRUE(fp->getLanguageSampleString("first").isEmpty());

    //构建特殊语言
    EXPECT_TRUE(fp->getLanguageSampleString("es_first") == "first");
    delete  fp;
}




