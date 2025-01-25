#ifndef LANGUAGECONFIG_H
#define LANGUAGECONFIG_H

#include <QString>
#include <QStringList>
#include <QMap>

// 结构体表示语言的能力，包括支持的编译器列表。
struct LanguageCapability {
    QStringList supportedCompilers;  // 支持的编译器列表
};

// LanguageConfig 类用于管理语言的配置和能力。
class LanguageConfig {
public:
    // 获取 LanguageConfig 的单例实例。
    static LanguageConfig& instance();
    
    // 获取支持的编译器列表。
    QStringList getSupportedCompilers(const QString& language) const;
    
    // 检查是否支持指定的语言。
    bool hasLanguage(const QString& language) const;

    // 构造函数，初始化语言配置。
    LanguageConfig();

private:
    QMap<QString, LanguageCapability> languageCapabilities; // 存储语言能力的映射
};

extern LanguageConfig g_instanceLanguageConfig; // 全局语言配置实例

#endif // LANGUAGECONFIG_H
