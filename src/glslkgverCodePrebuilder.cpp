#include "glslkgverCodePrebuilder.h"
#include <QRegularExpression>
#include "spirvUtils.h"

// 构造函数，初始化基础目录和包含路径
GlslKgverCodePrebuilder::GlslKgverCodePrebuilder(const QStringList &includePaths) 
    : includePaths(includePaths), includeDepth(0) {}

QString LoadCginc(const QString& strPath)
{
    std::string source;
	QString fullPath = QDir::currentPath() + "/" + strPath;
	FILE* fp = fopen(fullPath.toStdString().c_str(), "rb");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		unsigned char bomHeader[3];
		unsigned char dest[3] = {0xef, 0xbb, 0xbf};

		fread(bomHeader, sizeof(char) * 3, 1, fp);

		if (memcmp(bomHeader, dest, 3))
		{
			fseek(fp, 0, SEEK_SET);
		}
		else
		{
			len -= 3;
		}

	
		source.resize(len + 1, '\0');
		fread(&source[0], len, 1, fp);
		fclose(fp);
	}
	return QString(source.c_str());
}

QString LoadBaseMacroInc()
{
	return LoadCginc("external/glslkgver/macros.cginc");
}

// 解析着色器代码
QString GlslKgverCodePrebuilder::parse(const QString &shaderCode, const QString &startSection) {
    globalLineIter = 0;
    codeRecords.clear();
    content.clear();

    // 解析代码块
    initCodeSections(mainFile, shaderCode);
    content = parseCodeSections(mainFile, startSection, 0);

    QString contentBaseMacroInc = LoadBaseMacroInc() + "\n";
    addToHead(contentBaseMacroInc, "external/glslkgver/macros.cginc", "");

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
    filePath = filePath.toLower();

    // 检查是否为cginc文件
    if (filePath.endsWith(".cginc")) {
        QString fileName = filePath.mid(filePath.lastIndexOf('/') + 1);
        QString cgincPath = "external/glslkgver/" + fileName;

        QString cgincContent = LoadCginc(cgincPath) + "\n";
        QStringList lines = cgincContent.split('\n');

        AddCodeRecords(lines.size(), 1, cgincPath, "");
        return cgincContent;
    }

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

    int pushedNumLines = 0;
    int travelNumLines = 0;
    int travelLineOffset = 1;

    for (const QString &line : lines) {

        // 忽略以 @ 或 @@ 开头的行
        if (line.startsWith("@") || line.startsWith("@@")) {
            travelNumLines++;
            continue; // 跳过该行
        }

        if (line.startsWith("#include")) {
            if (pushedNumLines > 0)
            {
                AddCodeRecords(pushedNumLines, travelLineOffset, includeFile.filePath, startSection);
                pushedNumLines = 0;
            }

            QString includeContent = handleInclude(includeFile, line, depth + 1);
            if (!includeContent.isEmpty()) {
                processedLines.append(includeContent);
            } else{
                return "";
            }
        } else {
            if (pushedNumLines == 0)
            {
                travelLineOffset = travelNumLines;
            }

            pushedNumLines++;
            processedLines.append(line);
        }

        travelNumLines++;
    }

    if (pushedNumLines > 0)
    {
        AddCodeRecords(pushedNumLines, travelLineOffset, includeFile.filePath, startSection);
        pushedNumLines = 0;
        travelLineOffset = travelNumLines;
    }

    return processedLines.join('\n');
}

void GlslKgverCodePrebuilder::AddCodeRecords(int numLines, int sectionLocalLineOffset, const QString& IncludeFile, const QString& Section)
{
    if (numLines > 0)
    {
        CodeRecord rec = { IncludeFile, Section, globalLineIter, globalLineIter + numLines, sectionLocalLineOffset };
        codeRecords.emplace_back(rec);
        globalLineIter += numLines;
    }
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

void GlslKgverCodePrebuilder::addToHead(const QString& headCode, const QString& headFileName, const QString& sectionName)
{
    QStringList lines = headCode.split('\n');
    int numLines = lines.size();

    content = headCode + content;

    for (auto& rec : codeRecords)
    {
        rec.globalLineStart += numLines;
        rec.globalLineEnd += numLines;
    }

    globalLineIter += numLines;

    CodeRecord rec = { headFileName, sectionName, 1, numLines, 1 };
    codeRecords.insert(codeRecords.begin(), rec);
}

bool GlslKgverCodePrebuilder::matchGlobalLine(int globalLineNum, CodeFileLineInfo &retInfo)
{
    if (globalLineNum < 0)
        return false;

    for (const auto& rec : codeRecords)
    {
        if (globalLineNum >= rec.globalLineStart && globalLineNum < rec.globalLineEnd)
        {
            int includeSectionStart;
            QString lowIncludeFile = rec.IncludeFile.toLower();

            bool bCgincFile = lowIncludeFile.contains(".cginc");
            if (bCgincFile)
            {
                includeSectionStart = 0;
            }
            else
            {
                bool bFound = false;
                for (const auto& iter : includedFiles)
                {
                    if (iter.filePath == lowIncludeFile)
                    {
                        auto& codeSection = iter.codeSections[rec.Section];
                        includeSectionStart = codeSection.lineStart;
                        bFound = true;
                        break;
                    }
                }

                if (!bFound)
                    includeSectionStart = mainFile.codeSections[rec.Section].lineStart;
            }

            retInfo.includeFile = rec.IncludeFile;
            retInfo.inlineNum = globalLineNum - rec.globalLineStart + rec.sectionLocalLineOffset + includeSectionStart;
            return true;
        }
    }
    return false;
}

// 解析代码块
void GlslKgverCodePrebuilder::initCodeSections(CodeIncludeFile &includeFile, const QString &shaderCode)
{
    QStringList lines = shaderCode.split('\n');
    QString currentSectionName;
    QString currentSectionContent;
    int lineIter = 1;
    int lineStart = 1;
    int lineEnd = 1;

    for (const QString &line : lines) { 
        lineEnd++;
        lineIter++;

        // 跳过以@或@@开头的行
        if (line.startsWith("@") || line.startsWith("@@")) {
            continue;
        }

        if (line.startsWith("[")) {
            // 处理代码块定义
            if (!currentSectionName.isEmpty()) {
                // 存储之前的代码块
                includeFile.codeSections[currentSectionName] = {currentSectionName, currentSectionContent, lineStart, lineEnd };
            }
            currentSectionName = line.mid(1, line.length() - 2); // 获取代码块名称
            currentSectionContent.clear(); // 清空当前内容

            lineStart = lineIter;
            lineEnd = lineIter;
        } else {
            currentSectionContent.append(line + "\n"); // 添加到当前代码块内容
        }
    }

    if (!currentSectionName.isEmpty()) {
        // 存储最后一个代码块
        includeFile.codeSections[currentSectionName] = { currentSectionName, currentSectionContent, lineStart, lineEnd };
    } else if (!currentSectionContent.isEmpty() && includeFile.codeSections.empty()) {
        // 如果当前代码块内容不为空且包含文件没有代码块，则将当前代码块内容作为默认代码块
        includeFile.codeSections[""] = {"", currentSectionContent, lineStart, lineEnd};
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
        includeFileInstance.filePath = includeFile.fileName().toLower(); // 获取完整路径
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

QString TransformGlslKgverCodeErrors(GlslKgverCodePrebuilder &codePrebuilder, const QString& integrateCodeFileName, const QString& errorString)
{
    QStringList lines = errorString.split('\n');
    QString errorHeader = QString("ERROR: ") + integrateCodeFileName + QString(":");

    for (QString &line : lines)
    {
        bool isErrMsg = line.startsWith(errorHeader);
        if (isErrMsg)
        {
            QString errorMid = line.mid(errorHeader.length());
            QString globalLineNum = errorMid.mid(0, errorMid.indexOf(":")); // 提取行号
            QString errorContent = errorMid.mid(errorMid.indexOf(":") + 1); // 提取错误内容

            GlslKgverCodePrebuilder::CodeFileLineInfo errFileLineInfo;
            bool bret = codePrebuilder.matchGlobalLine(globalLineNum.toInt() - 4/*错误信息总会多4行？*/, errFileLineInfo);
            if (bret)
            {
                if (errFileLineInfo.includeFile.isEmpty())
                {
                    errFileLineInfo.includeFile = "textEditor";
                }
                line = QString("ERROR: %1(line: %2, global: %3)%4")
                    .arg(errFileLineInfo.includeFile)
                    .arg(QString::number(errFileLineInfo.inlineNum)) // 确保转换为字符串, 编辑器计数从1开始
                    .arg(globalLineNum)
                    .arg(errorContent);
            }
        }
    }
    return lines.join('\n');
}
