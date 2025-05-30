#include "compilerConfig.h"

// 全局编译器配置实例
CompilerConfig g_instanceCompilerConfig;

// 获取 CompilerConfig 的单例实例
CompilerConfig& CompilerConfig::instance()
{
    return g_instanceCompilerConfig;
}

// 获取指定编译器的能力
CompilerCapability CompilerConfig::getCapability(const QString& compiler) const {
    static CompilerCapability emptyCapability;  // 用于返回空配置
    if (!compilerCapabilities.contains(compiler)) {
        qDebug() << "Warning: Compiler" << compiler << "not found in configuration";
        return emptyCapability;
    }
    return compilerCapabilities[compiler];
}

// 检查是否支持指定的编译器
bool CompilerConfig::hasCompiler(const QString& compiler) const {
    return compilerCapabilities.contains(compiler);
}

// 构造函数，初始化编译器配置
CompilerConfig::CompilerConfig() {
    // FXC 编译器配置
    CompilerCapability fxc;
    fxc.supportedShaderTypes = QStringList() << "Vertex" << "Pixel" << "Geometry" << "Hull" << "Domain" << "Compute";
    fxc.supportedShaderModels = QStringList() << "5_0" << "5_1";
    fxc.supportedOutputTypes = QStringList() << "DXBC";
    compilerCapabilities["FXC"] = fxc;

    // DXC 编译器配置
    CompilerCapability dxc;
    dxc.supportedShaderTypes = QStringList() 
        << "Vertex" << "Pixel" << "Geometry" << "Hull" << "Domain" << "Compute"
        << "RayGeneration" << "RayIntersection" << "RayAnyHit" << "RayClosestHit"
        << "RayMiss" << "RayCallable" << "Amplification" << "Mesh";
    dxc.supportedShaderModels = QStringList() << "5_0" << "5_1" << "6_0" << "6_4" << "6_6" << "6_7";
    dxc.supportedOutputTypes = QStringList() << "DXIL" << "SPIR-V" << "GLSL" << "Preprocess-HLSL";
    compilerCapabilities["DXC"] = dxc;

    // GLSLANG 编译器配置
    CompilerCapability glslang;
    glslang.supportedShaderTypes = QStringList() 
        << "Vertex" << "Pixel" << "Geometry" << "TessControl" << "TessEvaluation" << "Compute"
        << "RayGeneration" << "RayIntersection" << "RayAnyHit" << "RayClosestHit"
        << "RayMiss" << "RayCallable" << "Task" << "Mesh";
    glslang.supportedShaderModels = QStringList() << "450" << "460";
    glslang.supportedOutputTypes = QStringList() << "SPIR-V" << "HLSL" << "GLSL";
    compilerCapabilities["GLSLANG"] = glslang;

    // GLSLANGKGVER 编译器配置
    CompilerCapability glslangkgver;
    glslangkgver.supportedShaderTypes = QStringList() 
        << "Vertex" << "Pixel" << "Compute";
    glslangkgver.supportedShaderModels = QStringList() << "450" << "460";
    glslangkgver.supportedOutputTypes = QStringList() << "SPIR-V" << "HLSL" << "GLSL";
    compilerCapabilities["GLSLANGKGVER"] = glslangkgver;
}
