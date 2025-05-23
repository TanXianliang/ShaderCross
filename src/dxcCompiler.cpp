#include "dxcCompiler.h"
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "spirvUtils.h"
#include <windows.h>

// 构造函数，初始化 dxcCompiler。
dxcCompiler::dxcCompiler(QObject *parent) : QObject(parent) {}

// 编译方法，执行编译操作。
void dxcCompiler::compile(const QString &shaderCode, 
                          const QString &languageType,
                          const QString &shaderModel, 
                          const QString &entryPoint,
                          const QString &shaderType,
                          const QString &outputType,
                          const QStringList &includePaths,
                          const QStringList &macros,
                          const QString &additionOptions) 
{
    // 使用临时文件来存储 Shader 代码
    QString tempFilePath = QDir::temp().filePath("temp_shader.hlsl");
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit compilationError("Failed to create temporary shader file.");
        return;
    }
    QTextStream out(&tempFile);
    out << shaderCode;  // 写入 Shader 代码
    tempFile.close();

    QString outputFilePath;
    if (outputType == "DXIL") {
        outputFilePath = QDir::temp().filePath("output_shader.dxil");
    }
    else if (outputType == "Preprocess-HLSL") {
        outputFilePath = QDir::temp().filePath("output_shader.hlsl");
    }
    else {
        outputFilePath = QDir::temp().filePath("output_shader.spv");
    }

    bool bHLSL2021 = false;
    if (languageType == "HLSL2021") {
        bHLSL2021 = true;
    }

    QFile::remove(outputFilePath);
    QString command = buildCommand(tempFilePath, shaderModel, entryPoint, shaderType, outputType, includePaths, macros, outputFilePath, bHLSL2021, additionOptions);

    LARGE_INTEGER Frequecy;
    QueryPerformanceFrequency(&Frequecy);

    double s_SecondsPerCPUCyscle = 1.0f / (double)Frequecy.QuadPart;

    LARGE_INTEGER BeginCircle;
    QueryPerformanceCounter(&BeginCircle);

    QProcess process;
    process.start(command);
    process.waitForFinished();

    LARGE_INTEGER EndCircle;
    QueryPerformanceCounter(&EndCircle);

    auto ToSec = s_SecondsPerCPUCyscle* (double)((int64_t)EndCircle.QuadPart - (int64_t)BeginCircle.QuadPart);

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (!QFile::exists(outputFilePath)) {
        emit compilationError(error.isEmpty() ? "Compilation failed with no output." : error);
    } else {
        QProcess process;
        QString errorDisasm;

        if (outputType == "DXIL"){
            // 使用dxc反编译DXIL
            QString dxilDisasmCommand = QString("dxc.exe -dumpbin \"%1\"").arg(outputFilePath);
            process.start(dxilDisasmCommand);
            process.waitForFinished();
            output = process.readAllStandardOutput();
            QString errorDisasm = process.readAllStandardError();
        } else if (outputType == "SPIR-V"){
            // 使用spirv-dis反编译SPIR-V
            QString spirvDisCommand = QString("spirv-dis.exe \"%1\"").arg(outputFilePath);
            process.start(spirvDisCommand);
            process.waitForFinished();
            output = process.readAllStandardOutput();
            QString errorDisasm = process.readAllStandardError();
        } else if (outputType == "GLSL"){
            // 使用spirv-cross将SPIR-V转换为GLSL
            QString spirvCrossCommand = QString("spirv-cross.exe \"%1\" -V").arg(outputFilePath);
            process.start(spirvCrossCommand);
            process.waitForFinished();
            output = process.readAllStandardOutput();
            QString errorDisasm = process.readAllStandardError();
        } else if (outputType == "Preprocess-HLSL") {
            QFile outFile(outputFilePath);
            if (!outFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                emit compilationError("Failed to create temporary shader file.");
                return;
            }
            QTextStream out(&outFile);
            output = out.readAll();  // 写入 Shader 代码
            outFile.close();
        }

        if (output.isEmpty()) {
            emit compilationError(errorDisasm.isEmpty() ? "Compilation failed with no output." : errorDisasm);
        } else {
            if (outputType == "SPIR-V") {
                QString outputReflectionInfo;
                if (DumpSpirVReflectionInfo(outputFilePath, outputReflectionInfo)) {
                    output = output + "\n" + outputReflectionInfo;
                }
            }

            emit compilationFinished(output + "\n" + QString("cost time: %1s").arg(ToSec));

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

QString dxcCompiler::buildCommand(const QString &tempFilePath, 
                                   const QString &shaderModel, 
                                   const QString &entryPoint,
                                   const QString &shaderType,
                                   const QString &outputType,
                                   const QStringList &includePaths,
                                   const QStringList &macros,
                                   const QString &outputFilePath,
                                   bool bHLSL2021,
                                   const QString &additionOptions) 
{
    // 基础命令
    QString command = "dxc.exe";
    
    // 添加着色器类型和模型
    QString stage;
    if (shaderType == "Vertex") stage = "vs";
    else if (shaderType == "Pixel") stage = "ps"; 
    else if (shaderType == "Geometry") stage = "gs";
    else if (shaderType == "Hull") stage = "hs";
    else if (shaderType == "Domain") stage = "ds";
    else if (shaderType == "Compute") stage = "cs";
    else if (shaderType == "RayGeneration") stage = "rgen";
    else if (shaderType == "RayIntersection") stage = "rint";
    else if (shaderType == "RayAnyHit") stage = "rahit";
    else if (shaderType == "RayClosestHit") stage = "rchit";
    else if (shaderType == "RayMiss") stage = "rmiss";
    else if (shaderType == "RayCallable") stage = "rcall";
    else if (shaderType == "Amplification") stage = "rs";
    else if (shaderType == "Mesh") stage = "ms";

    QString model = shaderModel;
    model.replace(".", "_");
    QString profile = stage + QString("_%1").arg(model);
    command += QString(" -T %1").arg(profile);
    
    // 添加入口点
    command += QString(" -E %1").arg(entryPoint);
    
    // 添加输出类型
    if (outputType == "SPIR-V" || outputType == "GLSL") {
        command += " -spirv";
    }

    if (outputType == "Preprocess-HLSL") {
        command += " -P";
    }

    if (!bHLSL2021) {
        command += " -HV 2016";
    }

    // 添加额外选项
    if (additionOptions.isEmpty() == false) 
        command += QString(" %1").arg(additionOptions);
    
    // 添加包含路径
    for (const QString &path : includePaths) {
        command += QString(" -I \"%1\"").arg(path);
    }
    
    // 添加宏定义
    for (const QString &macro : macros) {
        command += QString(" -D %1").arg(macro);
    }

    if (outputType != "Preprocess-HLSL") {
        command += QString(" -Fo \"%1\"").arg(outputFilePath);
    }
    else {
        command += QString(" -Fi \"%1\"").arg(outputFilePath);
    }

    // 添加输入文件
    command += QString(" \"%1\"").arg(tempFilePath);
    
    return command;
}