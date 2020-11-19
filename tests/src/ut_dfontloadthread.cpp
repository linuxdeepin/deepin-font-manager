#include "dfontloadthread.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include "dfontinfomanager.h"
#include "dsqliteutil.h"

#include <QFile>
#include <QDir>
#include <QSignalSpy>

namespace {
class TestDdFontloadthread : public testing::Test
{

protected:
    void SetUp()
    {
        flt = new DFontLoadThread;
    }
    void TearDown()
    {
    }
    // Some expensive resource shared by all tests.
    DFontLoadThread *flt;
};

}

TEST_F(TestDdFontloadthread, checkRun)
{
    QSignalSpy spy(flt, SIGNAL(loadFinished(QByteArray)));
    flt->open("");
    flt->run();
    EXPECT_EQ(1, spy.count());

    flt->open("/usr/share/fonts/truetype/noto/NotoSansNKo-Regular.ttf");
    flt->run();
    EXPECT_EQ(2, spy.count());
    delete flt;
}
