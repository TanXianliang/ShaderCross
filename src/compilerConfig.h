#ifndef COMPILERCONFIG_H
#define COMPILERCONFIG_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QDebug>

struct CompilerCapability {
    QStringList supportedShaderTypes;  // 支持的着色器类型
    QStringList supportedShaderModels; // 支持的Shader Model版本
    QStringList supportedOutputTypes;  // 支持的输出类型
};

class CompilerConfig {
public:
    static CompilerConfig& instance();
    
    CompilerCapability getCapability(const QString& compiler) const;
    bool hasCompiler(const QString& compiler) const;

    CompilerConfig();
    // 禁止拷贝和赋值
    CompilerConfig(const CompilerConfig&) = delete;
    CompilerConfig& operator=(const CompilerConfig&) = delete;

private:
    QMap<QString, CompilerCapability> compilerCapabilities;
};

extern CompilerConfig g_instanceCompilerConfig;

#endif // COMPILERCONFIG_H
