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

private slots:
    void onBrowseFile();
    void onCompile();
    void onSaveResult();
    void onShowDisassembly();
    void onResetLayout();
    void onToggleOutput();
    void onToggleTheme();
    void onAddIncludePath();
    void onRemoveIncludePath();
    void onAddMacro();
    void onRemoveMacro();
    void showMacroDialog();
    void onSaveSettings();

private:
    void createMenus();
    void applyTheme(bool dark);

    // 主界面UI组件
    QToolButton *closeButton;
    QToolButton *minButton;
    QToolButton *maxButton;  // 添加最大化按钮
    void updateButtonPositions();  // 添加新方法

    QPoint mouseQPoint;
    bool resizing;
    Qt::Edges resizeEdge;
    static const int RESIZE_MARGIN = 5;  // 边缘调整区域的宽度
    bool isDarkTheme;

private:
    // ShaderCode输入界面
    QLineEdit *filePathEdit;
    QPushButton *browseButton;
    ShaderCodeTextEdit *inputEdit;
    QComboBox *encodingCombo;
    QComboBox *languageCombo;

    QListWidget *includePathList;  // 包含路径
    QPushButton *addIncludeButton;
    QPushButton *removeIncludeButton;

    QListWidget *macroList;
    QPushButton *addMacroButton;
    QPushButton *removeMacroButton;
    
    // 编译输出界面
    QTextEdit *outputEdit;
    QTextEdit *logEdit;

    // 编译器设置
    CompilerSettingUI *compilerSettingUI;
    QPushButton *buildButton;  // 构建按钮

    // 界面操作记录
    QString lastOpenDir;
    QString lastHLSLCompiler;
    QString lastGLSLCompiler;

    void setupUI();
    void loadFileContent(const QString &fileName);
    void updateIncludeListHeight();
    void updateMacroListHeight();
    void loadSettings();
    QString settingsFilePath() const;
    void updateCurrentCompilerSettings(const QString &compiler);
};

#endif // MAINWINDOW_H 