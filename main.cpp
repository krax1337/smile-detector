#include "mainwindow.h"
#include <opencv2/opencv.hpp>
#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qApp->addLibraryPath("plugins");

    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    paths.append("sqldrivers");
    QCoreApplication::setLibraryPaths(paths);

    QApplication::setOrganizationName("Asanali_Bulatov");
    QApplication::setApplicationName("Smile_Detector");

    MainWindow w;
    w.show();

    return a.exec();
}
