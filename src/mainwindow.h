#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include "fxcCompiler.h"
#include "compilerSettingUI.h"
#include <QSettings>
#include "shaderCodeTextEdit.h"
#include "documentWindow.h"
#include <QTabWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 自定义关闭及最小化
    void windowclosed();
    void windowmin();
    //void paintEvent(QPaintEvent* event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void onNewDocument();
    void onNewDocumentByOpenWorkspace();
    void openShaderFile();  // 添加打开着色器
    void onSaveShaderFile();
    void onSaveWorkspace();
    void onResetLayout();
    void onToggleCurrentDocumentIncludePaths();
    void onToggleCurrentDocumentMacros();
    void onToggleTheme();
    void onTabCloseRequested(int index);
    void onTabMouseDoubleClickEvent(int tabIndex);

private:
    void createMenus();
    void applyTheme(bool dark);
    void setupUI();
    QString settingsFilePath() const;
    void loadSettings();    
    void saveSettings();
    DocumentWindow* getCurrentDocumentWindow();

    // 主界面UI组件
    QToolButton *closeButton = nullptr;
    QToolButton *minButton = nullptr;
    QToolButton *maxButton = nullptr;  // 添加最大化按钮
    void updateButtonPositions();  // 添加新方法

    QPoint mouseQPoint;
    bool resizing;
    Qt::Edges resizeEdge;
    static const int RESIZE_MARGIN = 5;  // 边缘调整区域的宽度
    bool isDarkTheme;

    QTabWidget *tabWidget; // 添加 Tab 控件
};

#endif // MAINWINDOW_H 