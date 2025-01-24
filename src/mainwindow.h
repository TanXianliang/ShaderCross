#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 移除这些函数
    // void mousePressEvent(QMouseEvent *event) override;
    // void mouseMoveEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;

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

private:
    // 输入设置控件
    QComboBox *languageCombo;
    QLineEdit *filePathEdit;
    QPushButton *browseButton;
    QTextEdit *includePathsEdit;
    QLineEdit *entryPointEdit;
    
    // 编译器选项
    QComboBox *compilerCombo;
    
    // 输出设置
    QComboBox *targetFormatCombo;
    QTextEdit *outputEdit;

    QTextEdit *inputEdit;  // 添加输入文本显示控件

    QComboBox *encodingCombo;  // 添加编码选择下拉框

    bool isDarkTheme;
    void applyTheme(bool dark);

    void setupUI();
    void createMenus();
    void setupShortcuts();

    void loadFileContent(const QString &fileName);  // 添加文件加载函数

    QListWidget *includePathList;  // 包含路径列表
    QPushButton *addIncludeButton;
    QPushButton *removeIncludeButton;

    QListWidget *macroList;  // 宏定义列表
    QPushButton *addMacroButton;
    QPushButton *removeMacroButton;
    void showMacroDialog();  // 显示添加宏的对话框

    void saveSettings();
    void loadSettings();
    QString settingsFilePath() const;  // 获取设置文件路径

    // 可能需要的其他编译器相关控件
    QComboBox *targetProfileCombo;  // shader模型选择

    QString lastHLSLCompiler;    // 记录HLSL最后使用的编译器
    QString lastGLSLCompiler;    // 记录GLSL最后使用的编译器

    QComboBox *shaderTypeCombo;  // shader类型选择

    QString lastOpenDir;    // 记录上次打开文件的目录

    QComboBox *shaderModelCombo;  // shader模型选择

    QComboBox *outputTypeCombo;  // 输出类型选择

    QPushButton *buildButton;  // 构建按钮

    QTextEdit *logEdit;  // 日志输出面板

    void updateIncludeListHeight();
    void updateMacroListHeight();
};

#endif // MAINWINDOW_H 