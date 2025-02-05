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
    void compilationFinished(const QString &output);
    void compilationError(const QString &error);

private:
    QString buildCommand(const QString &shaderCode, 
                        const QString &shaderModel, 
                        const QString &entryPoint,
                        const QString &shaderType,
                        const QStringList &includePaths,
                        const QStringList &macros);
};

#endif // FXCCOMPILER_H
