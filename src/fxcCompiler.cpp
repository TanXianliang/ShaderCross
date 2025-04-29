#include "fxcCompiler.h"
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <Windows.h>

fxcCompiler::fxcCompiler(QObject *parent) : QObject(parent) {}

void fxcCompiler::compile(
                        const QString &shaderCode, 
                        const QString &shaderModel, 
                        const QString &entryPoint, 
                        const QString &shaderType,
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

    QString outputFilePath = QDir::temp().filePath("output_shader.dxbc");
    QString command = buildCommand(tempFilePath, shaderModel, entryPoint, shaderType, includePaths, macros, outputFilePath, additionOptions);
    
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

    auto ToSec = s_SecondsPerCPUCyscle * (double)((int64_t)EndCircle.QuadPart - (int64_t)BeginCircle.QuadPart);

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    // 判断编译是否成功
    if (!QFile::exists(outputFilePath)) {
        emit compilationError(error.isEmpty() ? "Compilation failed with no output." : error);
    }
    else {
        QProcess process;

        QString dxilDisasmCommand = QString("fxc.exe -dumpbin \"%1\"").arg(outputFilePath);
        process.start(dxilDisasmCommand);

        process.waitForFinished();
        output = process.readAllStandardOutput();
        QString errorDisasm = process.readAllStandardError();

        if (output.isEmpty()) {
            emit compilationError(errorDisasm.isEmpty() ? "Compilation failed with no output." : errorDisasm);
        }
        else {
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

QString fxcCompiler::buildCommand(const QString &inputFile, 
                                   const QString &shaderModel, 
                                   const QString &entryPoint,
                                   const QString &shaderType,
                                   const QStringList &includePaths,
                                   const QStringList &macros,
                                   const QString &outputFilePath,
                                   const QString &additionOptions)
{
    // 检查着色器类型是否支持
    QStringList supportedTypes = {"Vertex", "Pixel", "Geometry", "Hull", "Domain", "Compute"};
    if (!supportedTypes.contains(shaderType)) {
        emit compilationError(QString("FXC compiler does not support %1 shader type").arg(shaderType));
        return QString();
    }
    
    // 基础命令
    QString command = "fxc.exe";
    
    // 添加着色器类型和模型
    QString model = shaderModel;
    model.replace(".", "_");
    QString profile = QString(shaderType.toLower().at(0)) + QString("s_%1").arg(model);
    command += QString(" /T %1").arg(profile);

    // 添加附加选项
    if (!additionOptions.isEmpty()) {
        command += QString(" %1").arg(additionOptions);
    }
    
    // 添加入口点
    command += QString(" /E %1").arg(entryPoint);
    
    // 添加包含路径
    for (const QString &path : includePaths) {
        command += QString(" /I \"%1\"").arg(path);
    }
    
    // 添加宏定义
    for (const QString &macro : macros) {
        command += QString(" /D %1").arg(macro);
    }

    if (!outputFilePath.isEmpty()) {
        command += QString(" /Fo %1").arg(outputFilePath);
    }

    // 添加输入文件
    command += QString(" \"%1\"").arg(inputFile);
    
    return command;
}