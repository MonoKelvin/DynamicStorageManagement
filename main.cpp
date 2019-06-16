#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile _file(":/qss/DSMStyle.qss");
    _file.open(QFile::ReadOnly);
    a.setStyleSheet(_file.readAll());
    _file.close();

    MainWindow w;
    w.show();

    return a.exec();
}
