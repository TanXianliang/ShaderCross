#ifndef DOCUMENTWINDOW_H
#define DOCUMENTWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include "shaderCodeTextEdit.h"
#include "compilerSettingUI.h"

class DocumentWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DocumentWindow(QWidget *parent = nullptr, const QString &documentTitle = "untitled");
    explicit DocumentWindow(QWidget *parent, const QString &documentTitle, const QString &workspaceSettingPath);
    ~DocumentWindow();

    void enableSave(bool enable);

public slots:
    void compile();
    void addIncludePath();
    void removeIncludePath();
    void addMacro();
    void removeMacro();
    void updateIncludeListHeight();
    void updateMacroListHeight();
    void updateCurrentCompilerSettings(const QString &compiler);
    void onBrowseFile();
    void loadFileContent(const QString &filePath);
    void undo();
    void redo();
    QString getContent();
    QString getEncoding();
    QString getLanguage();
    QString getDocumentWindowTitle() { return documentWindowTitle; }

public:
    void loadSettings(QString settingsPath);
    void saveSettings(QString settingsPath);

private:
    void setupUI();
    void setupConnections();
    QString settingsFilePath() const;

private:
    QString documentWindowTitle;

    // ShaderCode输入界面
    QLineEdit *filePathEdit;
    QPushButton *browseButton;
    QComboBox *languageCombo;
    QComboBox *encodingCombo;
    ShaderCodeTextEdit *inputEdit;

    QListWidget *includePathList; // 包含路径
    QPushButton *addIncludeButton;
    QPushButton *removeIncludeButton;

    QListWidget *macroList; // 宏定义
    QPushButton *addMacroButton;
    QPushButton *removeMacroButton;

    // 编译输出界面
    QTextEdit *outputEdit;
    QTextEdit *logEdit;

    // 编译器设置
    CompilerSettingUI *compilerSettingUI;

    // 界面操作记录
    QString lastHLSLCompiler;
    QString lastGLSLCompiler;
    QString lastOpenDir;

    bool isSaveSettings;
};

#endif
