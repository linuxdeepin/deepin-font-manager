
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

//TEST_F(TestDFontPreview, checkFontContainText)
//{
//    FT_Library m_library = nullptr;
//    FT_Face m_face = nullptr;
//    QString filepath = "/usr/share/fonts/fonts-cesi/CESI_KT_GB2312.TTF";
//    FT_Init_FreeType(&m_library);
//    int  error = FT_New_Face(m_library, filepath.toUtf8().constData(), 0, &m_face);

//    //断言可以正常获取到字体信息
//    EXPECT_EQ(true, error == 0);

//    //断言该字体可以显示中文字体
//    EXPECT_EQ(true, fp->checkFontContainText(m_face, FTM_DEFAULT_PREVIEW_CN_TEXT));
//    delete  fp;
//}

TEST_F(TestDFontPreview, checkPaintEvent)
{
    QRect r;
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();

    qDebug() << fp->fontDatabase.addApplicationFontFromData(fileContent);

    fp->setFileUrl(filepath);
    QPaintEvent *p = new QPaintEvent(r);
    fp->paintEvent(p);
    delete  fp;
}
