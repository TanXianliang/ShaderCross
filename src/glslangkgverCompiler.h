#ifndef GLSLANGKGVERCOMPILER_H
#define GLSLANGKGVERCOMPILER_H

#include <QString>
#include <QObject>
#include <QStringList>

// glslangkgverCompiler 类用于管理 glslangkgver 编译器的编译过程。
class glslangkgverCompiler : public QObject {
    Q_OBJECT

public:
    // 构造函数，初始化 glslangkgverCompiler.
    explicit glslangkgverCompiler(QObject *parent = nullptr);
    
    // 编译方法，接受 Shader 代码字符串
    void compile(const QString &shaderCode, 
                 const QString &shaderModel, 
                 const QString &entryPoint,
                 const QString &shaderType,
                 const QString &outputType,
                 const QStringList &includePaths,
                 const QStringList &macros);

signals:
    // 编译完成信号，携带输出结果。
    void compilationFinished(const QString &output);
    
    // 编译错误信号，携带错误信息。
    void compilationError(const QString &error);
    
    // 编译警告信号，携带警告信息。
    void compilationWarning(const QString &warning);

private:
    // 构建编译命令的方法。
    QString buildCommand(const QString &tempFilePath,  
                         const QString &shaderModel, 
                         const QString &shaderType,
                         const QString &outputType,
                         const QStringList &includePaths,
                         const QStringList &macros,
                         const QString &outputFilePath);
};

#endif // GLSLANGKGVERCOMPILER_H
