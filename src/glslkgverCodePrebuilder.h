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

private:
    QStringList includePaths; // 包含路径
    CodeIncludeFile mainFile; // 当前包含起始文件
    QMap<QString, CodeIncludeFile> includedFiles; // 包含的文件集合
    int includeDepth; // 当前包含深度
};

#endif // GLSLKGVERCODEPREBUILDER_H
