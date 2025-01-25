#ifndef COMPILERCONFIG_H
#define COMPILERCONFIG_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QDebug>

// 结构体表示编译器的能力，包括支持的着色器类型、模型和输出类型。
struct CompilerCapability {
    QStringList supportedShaderTypes;  // 支持的着色器类型
    QStringList supportedShaderModels; // 支持的 Shader Model 版本
    QStringList supportedOutputTypes;  // 支持的输出类型
};

// CompilerConfig 类用于管理编译器的配置和能力。
class CompilerConfig {
public:
    // 获取 CompilerConfig 的单例实例。
    static CompilerConfig& instance();
    
    // 获取指定编译器的能力。
    CompilerCapability getCapability(const QString& compiler) const;
    
    // 检查是否支持指定的编译器。
    bool hasCompiler(const QString& compiler) const;

    // 构造函数，初始化编译器配置。
    CompilerConfig();
    
    // 禁止拷贝和赋值
    CompilerConfig(const CompilerConfig&) = delete;
    CompilerConfig& operator=(const CompilerConfig&) = delete;

private:
    QMap<QString, CompilerCapability> compilerCapabilities; // 存储编译器能力的映射
};

extern CompilerConfig g_instanceCompilerConfig; // 全局编译器配置实例

#endif // COMPILERCONFIG_H
