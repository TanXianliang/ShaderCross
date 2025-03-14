#include "glslangkgverCompiler.h"
#include "glslkgverCodePrebuilder.h"
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "spirvUtils.h"

// 构造函数，初始化 glslangkgverCompiler
glslangkgverCompiler::glslangkgverCompiler(QObject *parent) : QObject(parent) {}

// 编译方法，执行编译操作。
void glslangkgverCompiler::compile(const QString &shaderCode,
                              const QString &shaderModel, 
                              const QString &entryPoint,
                              const QString &shaderType,
                              const QString &outputType,
                              const QStringList &includePaths,
                              const QStringList &macros, 
                              const QString &additionOptions)
{
    GlslKgverCodePrebuilder codePrebuilder(includePaths);
    QString combinedShaderCode = codePrebuilder.parse(shaderCode, entryPoint);
    QString shaderHeader;

    if (shaderType == "Vertex")
    {
        shaderHeader = 
            "#version 450\r\n"
            "#extension GL_ARB_separate_shader_objects : enable\r\n"
            "#extension GL_ARB_shading_language_420pack : enable\r\n"
            "#define SHADER_API 450\r\n";
    }
    else if (shaderType == "Compute")
    {
        shaderHeader = 
            "#version 450\r\n"
            "#extension GL_ARB_separate_shader_objects : enable\r\n"
            "#extension GL_ARB_shading_language_420pack : enable\r\n"
            "#define SHADER_API 450\r\n";
    }
    else if (shaderType == "Pixel" || shaderType == "Fragment")
    {
        shaderHeader = 
            "#version 450\r\n"
            "#extension GL_ARB_separate_shader_objects : enable\r\n"
            "#extension GL_ARB_shading_language_420pack : enable\r\n"
            "#define SHADER_API 450\r\n";
    }

    combinedShaderCode = shaderHeader + combinedShaderCode;

    // 使用临时文件来存储 Shader 代码
    QString tempFilePath = QDir::temp().filePath("temp_shader.tempcode");
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit compilationError("Failed to create temporary shader file.");
        return;
    }
    QTextStream out(&tempFile);
    out << combinedShaderCode;  // 写入 Shader 代码
    tempFile.close();

    QString outputFilePath = QDir::temp().filePath("output_shader.spv");

    QFile::remove(outputFilePath);
    QString command = buildCommand(tempFilePath, shaderModel, shaderType, outputType, includePaths, macros, outputFilePath, additionOptions);
    
    QProcess process;
    process.start(command);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    // 判断编译是否成功
    if (!QFile::exists(outputFilePath)) {
        if (!output.isEmpty())
        {
            error = TransformGlslKgverCodeErrors(codePrebuilder, tempFilePath, output);

            error = error + "\n" + codePrebuilder.getErrorLog();
            emit compilationError(error);
        }
        else
        {
            emit compilationError(error.isEmpty() ? "Compilation failed with no output." : error);
        }
    } else {
        QProcess process;

        // 优化spirv
        QString spirvOptCommand = QString("spirv-opt -O \"%1\" -o \"%1\"").arg(outputFilePath);
        process.start(spirvOptCommand);
        process.waitForFinished();

        if (outputType == "SPIR-V"){
            // 使用spirv-dis反编译SPIR-V
            QString spirvDisCommand = QString("spirv-dis.exe \"%1\"").arg(outputFilePath);
            process.start(spirvDisCommand);
        } else if (outputType == "GLSL"){
            // 使用spirv-cross将SPIR-V转换为GLSL
            QString spirvCrossCommand = QString("spirv-cross.exe \"%1\" -V").arg(outputFilePath);
            process.start(spirvCrossCommand);
        }
        else if (outputType == "HLSL"){
            // 使用spirv-cross将SPIR-V转换为HLSL
            QString spirvCrossCommand = QString("spirv-cross.exe \"%1\" --hlsl --shader-model 60").arg(outputFilePath);
            process.start(spirvCrossCommand);
        }

        process.waitForFinished();
        output = process.readAllStandardOutput();
        QString errorDisasm = process.readAllStandardError();

        if (output.isEmpty()) {
            emit compilationError(errorDisasm.isEmpty() ? "Compilation failed with no output." : errorDisasm);
        } else {
            if (outputType == "SPIR-V") {
                QString outputReflectionInfo;
                if (DumpSpirVReflectionInfo(outputFilePath, outputReflectionInfo)) {
                    output = output + "\n" + outputReflectionInfo;
                }
            }

            emit compilationFinished(output);

            // 如果 error 非空，将其输出为警告信息
            if (!error.isEmpty()) {
                emit compilationWarning(error);  // 直接发出错误信号
            }

            if (!errorDisasm.isEmpty()) {
                emit compilationWarning(errorDisasm);
            }
        }
    }

    // 删除临时文件
    QFile::remove(tempFilePath);
    QFile::remove(outputFilePath);
}

QString glslangkgverCompiler::buildCommand(
                                      const QString &tempFilePath, 
                                      const QString &shaderModel, 
                                      const QString &shaderType,
                                      const QString &outputType,
                                      const QStringList &includePaths,
                                      const QStringList &macros,
                                      const QString &outputFilePath, 
                                      const QString &additionOptions
                                      )
{
    // 基础命令
    QString command = "glslangValidator";

    // 添加着色器类型和模型
    QString stage;
    if (shaderType == "Vertex") stage = "vert";
    else if (shaderType == "Pixel") stage = "frag"; 
    else if (shaderType == "Geometry") stage = "geom";
    else if (shaderType == "TessControl") stage = "tesc";
    else if (shaderType == "TessEvaluation") stage = "tese";
    else if (shaderType == "Compute") stage = "comp";
    else if (shaderType == "RayGeneration") stage = "rgen";
    else if (shaderType == "RayIntersection") stage = "rint";
    else if (shaderType == "RayAnyHit") stage = "rahit";
    else if (shaderType == "RayClosestHit") stage = "rchit";
    else if (shaderType == "RayMiss") stage = "rmiss";
    else if (shaderType == "RayCallable") stage = "rcall";
    else if (shaderType == "Task") stage = "task";
    else if (shaderType == "Mesh") stage = "mesh";

    command += QString(" -S %1").arg(stage); // 根据glslang规范转换着色器类型

    // 自动绑定uniform变量
    command += " --auto-map-bindings";
    command += " --auto-map-locations";

    if (!additionOptions.isEmpty()) {
        command += " " + additionOptions;
    }

    // 指定输出文件
    command += QString(" -V -o \"%1\"").arg(outputFilePath);

    // 添加包含路径
    for (const QString &path : includePaths) {
        command += QString(" -I\"%1\"").arg(path);
    }
    
    // 添加宏定义
    for (const QString &macro : macros) {
        command += QString(" --D %1").arg(macro);
    }
    
    // 添加输入文件
    command += QString(" \"%1\"").arg(tempFilePath);

    return command;
}
