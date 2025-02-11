#ifndef GLSLKGVERCODEPREBUILDER_H
#define GLSLKGVERCODEPREBUILDER_H

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QMap>

class GlslKgverCodePrebuilder;

class CodeSection {
public:
    QString name; // 代码块名称
    QString content; // 代码块内容
    int lineStart; // 行号从1开始
    int lineEnd; // 结束行号等于lineEnd - 1，lineNum = lineEnd - lineStart
};

class CodeIncludeFile
{
public:
    QMap<QString, CodeSection> codeSections; // 存储代码块
    QString filePath; // 包含文件路径
};

class GlslKgverCodePrebuilder {
public:
    GlslKgverCodePrebuilder(const QStringList &includePaths);
    
    // 解析着色器代码
    QString parse(const QString &shaderCode, const QString &startSection);

    struct CodeFileLineInfo
    {
        QString includeFile;
        int inlineNum;
    };
    bool matchGlobalLine(int globalLineNum, CodeFileLineInfo& retInfo);

    QString getErrorLog() const { return error; }

private:
    // 初始化包含代码文件
    void initCodeSections(CodeIncludeFile &includeFile, const QString &shaderCode);

    // 获取包含文件
    CodeIncludeFile getIncludeFile(const QString &filePath);

    // 处理 #include 指令
    QString handleInclude(const CodeIncludeFile &currentFile, const QString &line, int depth);

    // 解析代码块
    QString parseCodeSections(const CodeIncludeFile &includeFile, const QString &startSection, int depth);

    // 替换autobind
    QString replaceAutoBind(const QString& shaderCode);

    // headCode添加到代码前端
    void addToHead(const QString& headCode, const QString& headFileName, const QString& sectionName);

    void errorLog(const QString& errorLog);

private:
    QString content;
    QString error;
    QStringList includePaths; // 包含路径
    CodeIncludeFile mainFile; // 当前包含起始文件
    QMap<QString, CodeIncludeFile> includedFiles; // 包含的文件集合
    int includeDepth; // 当前包含深度

    struct CodeRecord
    {
        QString IncludeFile;
        QString Section;
        int globalLineStart; // 行号从1开始
        int globalLineEnd; // 结束行号等于lineEnd - 1，lineNum = lineEnd - lineStart
        int sectionLocalLineOffset; // 在CodeSection中的局部行号，从1开始
    };
    std::vector<CodeRecord> codeRecords;
    int globalLineIter;

    void AddCodeRecords(int numLines, int sectionLocalLineOffset, const QString &IncludeFile, const QString &Section);
};

QString TransformGlslKgverCodeErrors(GlslKgverCodePrebuilder& codePrebuilder, const QString& integrateCodeFileName, const QString& errorString);

#endif // GLSLKGVERCODEPREBUILDER_H
