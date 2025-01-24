#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序图标
    app.setWindowIcon(QIcon(":/icons/icons/icon.jpg"));
    
    MainWindow w;
    w.show();
    
    return app.exec();
} 