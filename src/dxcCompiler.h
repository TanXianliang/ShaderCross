#ifndef DXCCOMPILER_H
#define DXCCOMPILER_H

#include <QString>
#include <QObject>
#include <QStringList>

// dxcCompiler 类用于管理 DXC 编译器的编译过程。
class dxcCompiler : public QObject {
    Q_OBJECT

public:
    // 构造函数，初始化 dxcCompiler。
    explicit dxcCompiler(QObject *parent = nullptr);
    
    // 编译方法，执行编译操作。
    void compile(const QString &inputFile, 
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
    QString buildCommand(const QString &inputFile, 
                         const QString &shaderModel, 
                         const QString &entryPoint,
                         const QString &shaderType,
                         const QString &outputType,
                         const QStringList &includePaths,
                         const QStringList &macros);
};

#endif // DXCCOMPILER_H
