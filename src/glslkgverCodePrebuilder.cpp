#include "glslkgverCodePrebuilder.h"

// 构造函数，初始化基础目录和包含路径
GlslKgverCodePrebuilder::GlslKgverCodePrebuilder(const QStringList &includePaths) 
    : includePaths(includePaths), includeDepth(0) {}

// 解析着色器代码
QString GlslKgverCodePrebuilder::parse(const QString &shaderCode, const QString &startSection) {
    // 解析代码块
    initCodeSections(mainFile, shaderCode);
    QString content = parseCodeSections(mainFile, startSection, 0);

    content = replaceAutoBind(content);
    return content;
}

// 处理 #include 指令
QString GlslKgverCodePrebuilder::handleInclude(const CodeIncludeFile &currentFile, const QString &line, int depth) {
    if (depth > 100) {
        qWarning() << "Include depth exceeded 100, aborting to prevent circular includes.";
        return ""; // 返回空字符串表示无法处理
    }

    QStringList parts = line.split(' ');
    if (parts.size() < 2) {
        return ""; // 无效的 include 指令
    }

    QString filePath = parts[1].mid(parts[1].indexOf('"') + 1, parts[1].lastIndexOf('"') - parts[1].indexOf('"') - 1);
    filePath.toLower();

    QString sectionName;
    if (parts.size() > 2) {
        sectionName = parts[2]; // 获取代码块名称
        int startIndex = sectionName.indexOf("[");
        int endIndex = sectionName.lastIndexOf("]");
        if (startIndex != -1 && endIndex != -1 && startIndex < endIndex) {
            sectionName = sectionName.mid(startIndex + 1, endIndex - startIndex - 1); // 去掉[]
        }
    }

    CodeIncludeFile includeFile;
    if (filePath == "self") {
        includeFile = currentFile;
    }
    else{
        includeFile = getIncludeFile(filePath);
    }

    if (includeFile.codeSections.isEmpty()) {
        qWarning() << "Failed to parse include file:" << filePath;
        return ""; // 返回空字符串表示无法处理
    } else {
        return parseCodeSections(includeFile, sectionName, depth + 1);
    }
}

QString GlslKgverCodePrebuilder::parseCodeSections(const CodeIncludeFile &includeFile, const QString &startSection, int depth)
{
    QStringList lines = includeFile.codeSections[startSection].content.split('\n');
    QStringList processedLines;

    for (const QString &line : lines) {
        // 忽略以 @ 或 @@ 开头的行
        if (line.startsWith("@") || line.startsWith("@@")) {
            continue; // 跳过该行
        }

        if (line.startsWith("#include")) {
            QString includeContent = handleInclude(includeFile, line, depth + 1);
            if (!includeContent.isEmpty()) {
                processedLines.append(includeContent);
            } else{
                return "";
            }
        } else {
            processedLines.append(line);
        }
    }

    return processedLines.join('\n');
}

QString GlslKgverCodePrebuilder::replaceAutoBind(const QString &shaderCode)
{
    QStringList lines = shaderCode.split('\n');

    // 替换auto_bind为递增序号
    int bindIndex = 0;

    bool inMacro = false;
    for (QString &line : lines) {
        // 检查是否在多行宏定义中
        if (line.startsWith("#")) {
            inMacro = line.trimmed().endsWith("\\");
            continue;
        }
        // 如果还在多行宏中则跳过
        if (inMacro) {
            inMacro = line.trimmed().endsWith("\\");
            continue;
        }

        if (line.contains("auto_bind")) {
            line.replace("auto_bind", QString::number(bindIndex));
            bindIndex++;
        }
    }

    return lines.join('\n');
}

// 解析代码块
void GlslKgverCodePrebuilder::initCodeSections(CodeIncludeFile &includeFile, const QString &shaderCode)
{
    QStringList lines = shaderCode.split('\n');
    QString currentSectionName;
    QString currentSectionContent;

    for (const QString &line : lines) {
        // 跳过以@或@@开头的行
        if (line.startsWith("@") || line.startsWith("@@")) {
            continue;
        }

        if (line.startsWith("[")) {
            // 处理代码块定义
            if (!currentSectionName.isEmpty()) {
                // 存储之前的代码块
                includeFile.codeSections[currentSectionName] = {currentSectionName, currentSectionContent};
            }
            currentSectionName = line.mid(1, line.length() - 2); // 获取代码块名称
            currentSectionContent.clear(); // 清空当前内容
        } else {
            currentSectionContent.append(line + "\n"); // 添加到当前代码块内容
        }
    }

    if (!currentSectionName.isEmpty()) {
        // 存储最后一个代码块
        includeFile.codeSections[currentSectionName] = { currentSectionName, currentSectionContent };
    } else if (!currentSectionContent.isEmpty() && includeFile.codeSections.empty()) {
        // 如果当前代码块内容不为空且包含文件没有代码块，则将当前代码块内容作为默认代码块
        includeFile.codeSections[""] = {"", currentSectionContent};
    }
}

// 获取包含文件
CodeIncludeFile GlslKgverCodePrebuilder::getIncludeFile(const QString &filePath)
{
    if (includedFiles.contains(filePath)) {
        return includedFiles[filePath];
    }

    QFile includeFile;
    if (QDir::isAbsolutePath(filePath)) {
        // 如果是绝对路径,直接打开文件
        includeFile.setFileName(filePath);
    } else {
        // 在多个包含路径中查找文件
        for (const QString &includePath : includePaths) {
            includeFile.setFileName(QDir(includePath).filePath(filePath));
            if (includeFile.exists()) {
                break;
            }
        }
    }

    if (includeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&includeFile);
        QString content;
        while (!in.atEnd()) {
            QString includeLine = in.readLine();
            content.append(includeLine + "\n"); // 添加有效行
        }
        includeFile.close();

        CodeIncludeFile includeFileInstance;
        includeFileInstance.filePath = includeFile.fileName(); // 获取完整路径
        initCodeSections(includeFileInstance, content);
        if (includeFileInstance.codeSections.isEmpty()) {
            qWarning() << "Failed to parse include file:" << includeFile.fileName();
            return CodeIncludeFile(); // 返回空字符串表示无法处理
        }
        includedFiles[filePath] = includeFileInstance;
        return includeFileInstance;
    }

    qWarning() << "Failed to open include file:" << filePath;
    return CodeIncludeFile(); // 返回空字符串表示无法处理
}
