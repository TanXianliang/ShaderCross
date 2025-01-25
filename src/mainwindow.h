#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include "fxcCompiler.h"
#include "compilerSettingUI.h"
#include <QSettings>

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
    void showMacroDialog();
    void onSaveSettings();

private:
    // UI 组件
    QLineEdit *filePathEdit;
    QPushButton *browseButton;
    QTextEdit *inputEdit;
    QTextEdit *outputEdit;
    QTextEdit *logEdit;
    QComboBox *encodingCombo;
    QComboBox *languageCombo;
    
    QListWidget *includePathList;
    QPushButton *addIncludeButton;
    QPushButton *removeIncludeButton;

    QListWidget *macroList;
    QPushButton *addMacroButton;
    QPushButton *removeMacroButton;

    CompilerSettingUI *compilerSettingUI;  // 新增编译器设置UI
    fxcCompiler *compiler;

    bool isDarkTheme;
    QString lastOpenDir;
    QString lastHLSLCompiler;
    QString lastGLSLCompiler;

    void setupUI();
    void createMenus();
    void setupShortcuts();
    void loadFileContent(const QString &fileName);
    void applyTheme(bool dark);
    void updateIncludeListHeight();
    void updateMacroListHeight();
    void loadSettings();
    QString settingsFilePath() const;

    // 可能需要的其他编译器相关控件
    QComboBox *targetProfileCombo;  // shader模型选择
    QComboBox *shaderTypeCombo;  // shader类型选择
    QComboBox *shaderModelCombo;  // shader模型选择
    QComboBox *outputTypeCombo;  // 输出类型选择
    QPushButton *buildButton;  // 构建按钮

    // 新增方法声明
    void updateCurrentCompilerSettings(const QString &compiler);
};

#endif // MAINWINDOW_H 