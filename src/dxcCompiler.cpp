#include "dxcCompiler.h"
#include <QProcess>
#include <QDebug>

dxcCompiler::dxcCompiler(QObject *parent) : QObject(parent) {}

void dxcCompiler::compile(const QString &inputFile, 
                          const QString &shaderModel, 
                          const QString &entryPoint,
                          const QString &shaderType,
                          const QString &outputType,
                          const QStringList &includePaths,
                          const QStringList &macros) 
{
    QString command = buildCommand(inputFile, shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    
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

QString dxcCompiler::buildCommand(const QString &inputFile, 
                                   const QString &shaderModel, 
                                   const QString &entryPoint,
                                   const QString &shaderType,
                                   const QString &outputType,
                                   const QStringList &includePaths,
                                   const QStringList &macros) 
{
    // 检查输出类型是否支持
    QStringList supportedOutputTypes = {"DXIL", "SPIR-V"};
    if (!supportedOutputTypes.contains(outputType)) {
        emit compilationError(QString("DXC compiler only supports output types: %1").arg(supportedOutputTypes.join(", ")));
        return QString();
    }
    
    // 基础命令
    QString command = "dxc.exe";
    
    // 添加着色器类型和模型
    QString model = shaderModel;
    model.replace(".", "_");
    QString profile = QString(shaderType.toLower().at(0)) + QString("s_%1").arg(model);
    command += QString(" -T %1").arg(profile);
    
    // 添加入口点
    command += QString(" -E %1").arg(entryPoint);
    
    // 添加输出类型
    if (outputType == "SPIR-V") {
        command += " -spirv";
    }
    
    // 添加包含路径
    for (const QString &path : includePaths) {
        command += QString(" -I \"%1\"").arg(path);
    }
    
    // 添加宏定义
    for (const QString &macro : macros) {
        command += QString(" -D %1").arg(macro);
    }
    
    // 添加输入文件
    command += QString(" \"%1\"").arg(inputFile);
    
    return command;
}
