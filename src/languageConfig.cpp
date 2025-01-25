#include "languageConfig.h"

// 全局语言配置实例
LanguageConfig g_instanceLanguageConfig;

// 获取 LanguageConfig 的单例实例
LanguageConfig& LanguageConfig::instance()
{
    return g_instanceLanguageConfig;
}

// 获取支持的编译器列表
QStringList LanguageConfig::getSupportedCompilers(const QString& language) const {
    if (!languageCapabilities.contains(language)) {
        return QStringList();
    }
    return languageCapabilities[language].supportedCompilers;
}

// 检查是否支持指定的语言
bool LanguageConfig::hasLanguage(const QString& language) const {
    return languageCapabilities.contains(language);
}

// 构造函数，初始化语言配置
LanguageConfig::LanguageConfig() {
    // HLSL 语言配置 - 支持所有编译器
    LanguageCapability hlsl;
    hlsl.supportedCompilers = QStringList() << "FXC" << "DXC" << "GLSLANG" << "SPIRV-CROSS";
    languageCapabilities["HLSL"] = hlsl;

    // GLSL 语言配置
    LanguageCapability glsl;
    glsl.supportedCompilers = QStringList() << "GLSLANG" << "SPIRV-CROSS";
    languageCapabilities["GLSL"] = glsl;
}
