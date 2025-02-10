#ifndef FXCCOMPILER_H
#define FXCCOMPILER_H

#include <QString>
#include <QObject>
#include <QStringList>

class fxcCompiler : public QObject {
    Q_OBJECT

public:
    explicit fxcCompiler(QObject *parent = nullptr);
    void compile(const QString &shaderCode, 
                const QString &shaderModel, 
                const QString &entryPoint,
                const QString &shaderType,
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
    QString buildCommand(const QString &shaderCode, 
                        const QString &shaderModel, 
                        const QString &entryPoint,
                        const QString &shaderType,
                        const QStringList &includePaths,
                        const QStringList &macros,
                        const QString &outputFilePath);
};

#endif // FXCCOMPILER_H
