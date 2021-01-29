
#include "dcopyfilesmanager.h"

#include "dfontmanager.h"
#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QStandardPaths>
#include <QApplication>
#include <QThreadPool>
#include <QSignalSpy>
#include <QDir>
#include <QDebug>

namespace {
class TestDCopyFilesManager : public testing::Test
{

protected:
    void SetUp()
    {
        fmd = new DCopyFilesManager();
    }
    void TearDown()
    {
//        delete  fmd;
    }

    // Some expensive resource shared by all tests.
    DCopyFilesManager *fmd;
};

QString stub_getTargetPath(const QString &inPath, QString &srcPath, QString &targetPath)
{
    Q_UNUSED(inPath);
    Q_UNUSED(srcPath);
    Q_UNUSED(targetPath);

    return QString();

}

}

TEST_F(TestDCopyFilesManager, checkRunInstall)
{
    CopyFontThread *t = new CopyFontThread(CopyFontThread::INSTALL, 0);

    QSignalSpy spy(t, SIGNAL(fileInstalled(const QString &, const QString &)));

    t->m_srcFiles.clear();
    t->run();

    Stub s;
    s.set(ADDR(DCopyFilesManager, getTargetPath), stub_getTargetPath);
    t->appendFile("first");
    DCopyFilesManager::m_installCanceled = false;
    t->run();

    qDebug() << spy.count() << endl;

    EXPECT_TRUE(spy.count() == 1);
    delete  t;
}

TEST_F(TestDCopyFilesManager, checkRunExport)
{
    CopyFontThread *t = new CopyFontThread(CopyFontThread::EXPORT, 0);
    t->appendFile("first");
    t->run();
    t->deleteLater();
}


TEST_F(TestDCopyFilesManager, checkCopyFilesNULL)
{
    QStringList list;
    fmd->copyFiles(CopyFontThread::INSTALL, list);
}

TEST_F(TestDCopyFilesManager, checkCopyFilesLess)
{
    QStringList filelist;
    filelist << "first";

    Stub s;
    s.set(ADDR(DCopyFilesManager, getTargetPath), stub_getTargetPath);

    fmd->copyFiles(CopyFontThread::INVALID, filelist);
}

//getTargetPath
TEST_F(TestDCopyFilesManager, checkGetTargetPath)
{
    QString bstr;
    QString astr;
    QString familyname;

    familyname = fmd->getTargetPath("1|2|3", astr, bstr);
    EXPECT_TRUE(familyname == "2");

    familyname = fmd->getTargetPath("1|2/|3", astr, bstr);
    EXPECT_TRUE(familyname == "2-");

    familyname = fmd->getTargetPath("1||3", astr, bstr);
    EXPECT_TRUE(familyname == "");
}

TEST_F(TestDCopyFilesManager, checkDeleteFiles)
{
    QStringList filelist;
    filelist << "1|2|3";

    fmd->deleteFiles(filelist, false);
}








