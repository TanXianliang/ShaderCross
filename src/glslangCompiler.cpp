#include "glslangCompiler.h"
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>

// 构造函数，初始化 glslangCompiler。
glslangCompiler::glslangCompiler(QObject *parent) : QObject(parent) {}

// 编译方法，执行编译操作。
void glslangCompiler::compile(const QString &shaderCode, 
                              const QString &languageType,
                              const QString &shaderModel, 
                              const QString &entryPoint,
                              const QString &shaderType,
                              const QString &outputType,
                              const QStringList &includePaths,
                              const QStringList &macros)
{
    // 使用临时文件来存储 Shader 代码
    QString tempFilePath = QDir::temp().filePath("temp_shader.tempcode");
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit compilationError("Failed to create temporary shader file.");
        return;
    }
    QTextStream out(&tempFile);
    out << shaderCode;  // 写入 Shader 代码
    tempFile.close();

    bool isHLSL = languageType == "HLSL";

    QString outputFilePath = QDir::temp().filePath("output_shader.spv");

    QFile::remove(outputFilePath);
    QString command = buildCommand(tempFilePath, isHLSL, shaderModel, entryPoint, shaderType, outputType, includePaths, macros, outputFilePath);
    
    QProcess process;
    process.start(command);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    // 判断编译是否成功
    if (!QFile::exists(outputFilePath)) {
        if (!output.isEmpty())
            emit compilationError(output);
        else
            emit compilationError(error.isEmpty() ? "Compilation failed with no output." : error);
    } else {
        QProcess process;

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

QString glslangCompiler::buildCommand(const QString &tempFilePath, 
                                      bool isHLSL,
                                      const QString &shaderModel, 
                                      const QString &entryPoint,
                                      const QString &shaderType,
                                      const QString &outputType,
                                      const QStringList &includePaths,
                                      const QStringList &macros,
                                      const QString &outputFilePath)
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

    if (isHLSL) {
        // 添加入口点
        command += QString(" -D -e %1 --hlsl-enable-16bit-types").arg(entryPoint); 
    }

    // 自动绑定uniform变量
    command += " --amb";
    
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
