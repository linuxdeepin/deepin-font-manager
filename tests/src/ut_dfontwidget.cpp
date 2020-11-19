#include "dfontwidget.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <QFile>
#include <QTimer>
#include <QTranslator>
#include <QDebug>

namespace {
class TestDFontWidget : public testing::Test
{
protected:
    void SetUp()
    {
        fw = new DFontWidget();
    }
    void TearDown()
    {
        delete  fw;
    }
    // Some expensive resource shared by all tests.
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    DFontWidget *fw;
};
}

TEST_F(TestDFontWidget, checkSetFileUrl)
{
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    fw->setFileUrl(filepath);
    EXPECT_EQ(true, fw->m_spinner->isPlaying());
    EXPECT_EQ(true, fw->m_layout->currentIndex() == 0);
    EXPECT_EQ(true, fw->m_filePath == filepath);
    fw->m_thread->quit();
    fw->m_thread->wait();
}

TEST_F(TestDFontWidget, checkHandleFinished_dataEmpty)
{
    QByteArray data;
//    fw->setFileUrl(filepath);
    fw->handleFinished(data);
    EXPECT_EQ(true, fw->m_layout->currentIndex() == 0);
    EXPECT_EQ(false, fw->m_preview->isVisible());
}

TEST_F(TestDFontWidget, checkHandleFinished_data)
{
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();
//    fw->setFileUrl(filepath);
    fw->handleFinished(fileContent);
    EXPECT_EQ(true, fw->m_errMsg->text() == DApplication::translate("DFontWidget", "Broken file"));
//    EXPECT_EQ(true, fw->m_preview->isVisible());
}
