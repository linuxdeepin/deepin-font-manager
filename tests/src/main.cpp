#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    qDebug() << "start libdeepin-font-manager test cases ..............";

    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication application(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end libdeepin-font-manager test cases ..............";
    return ret;
}
