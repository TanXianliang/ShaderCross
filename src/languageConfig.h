#ifndef LANGUAGECONFIG_H
#define LANGUAGECONFIG_H

#include <QString>
#include <QStringList>
#include <QMap>

struct LanguageCapability {
    QStringList supportedCompilers;  // 支持的编译器列表
};

class LanguageConfig {
public:
    static LanguageConfig& instance();
    
    // 修改返回类型，返回副本而不是引用
    QStringList getSupportedCompilers(const QString& language) const;
    bool hasLanguage(const QString& language) const;

    LanguageConfig();

private:
    QMap<QString, LanguageCapability> languageCapabilities;
};

extern LanguageConfig g_instanceLanguageConfig;

#endif // LANGUAGECONFIG_H
