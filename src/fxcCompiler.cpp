#include "fxcCompiler.h"
#include <QProcess>
#include <QDebug>

fxcCompiler::fxcCompiler(QObject *parent) : QObject(parent) {}

void fxcCompiler::compile(const QString &inputFile, 
                         const QString &shaderModel, 
                         const QString &entryPoint,
                         const QString &shaderType,
                         const QStringList &includePaths,
                         const QStringList &macros) 
{
    QString command = buildCommand(inputFile, shaderModel, entryPoint, shaderType, includePaths, macros);
    
    QProcess process;
    process.start(command);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (error.isEmpty()) {
        emit compilationFinished(output);
    } else {
        emit compilationError(error);
    }
}

QString fxcCompiler::buildCommand(const QString &inputFile, 
                                const QString &shaderModel, 
                                const QString &entryPoint,
                                const QString &shaderType,
                                const QStringList &includePaths,
                                const QStringList &macros) 
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
    
    // 添加输入文件
    command += QString(" \"%1\"").arg(inputFile);
    
    return command;
}