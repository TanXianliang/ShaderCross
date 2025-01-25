#include "languageConfig.h"

LanguageConfig g_instanceLanguageConfig;

LanguageConfig& LanguageConfig::instance()
{
    return g_instanceLanguageConfig;
}

QStringList LanguageConfig::getSupportedCompilers(const QString& language) const {
    if (!languageCapabilities.contains(language)) {
        return QStringList();
    }
    return languageCapabilities[language].supportedCompilers;
}

bool LanguageConfig::hasLanguage(const QString& language) const {
    return languageCapabilities.contains(language);
}

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
