#include <QApplication>
#include "mainwindow.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序图标
    app.setWindowIcon(QIcon(":/resources/icons/icon.jpg"));
    
    MainWindow w;
    w.show();
    
    return app.exec();
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
#endif 