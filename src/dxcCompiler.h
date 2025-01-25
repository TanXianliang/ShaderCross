#ifndef DXCCOMPILER_H
#define DXCCOMPILER_H

#include <QString>
#include <QObject>
#include <QStringList>

class dxcCompiler : public QObject {
    Q_OBJECT

public:
    explicit dxcCompiler(QObject *parent = nullptr);
    void compile(const QString &inputFile, 
                 const QString &shaderModel, 
                 const QString &entryPoint,
                 const QString &shaderType,
                 const QString &outputType,
                 const QStringList &includePaths,
                 const QStringList &macros);

signals:
    void compilationFinished(const QString &output);
    void compilationError(const QString &error);
    void compilationWarning(const QString &warning);

private:
    QString buildCommand(const QString &inputFile, 
                         const QString &shaderModel, 
                         const QString &entryPoint,
                         const QString &shaderType,
                         const QString &outputType,
                         const QStringList &includePaths,
                         const QStringList &macros);
};

#endif // DXCCOMPILER_H
