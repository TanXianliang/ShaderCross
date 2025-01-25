#include "compilerConfig.h"

CompilerConfig g_instanceCompilerConfig;

CompilerConfig& CompilerConfig::instance()
{
    return g_instanceCompilerConfig;
}

CompilerCapability CompilerConfig::getCapability(const QString& compiler) const {
    static CompilerCapability emptyCapability;  // 用于返回空配置
    if (!compilerCapabilities.contains(compiler)) {
        qDebug() << "Warning: Compiler" << compiler << "not found in configuration";
        return emptyCapability;
    }
    return compilerCapabilities[compiler];
}

bool CompilerConfig::hasCompiler(const QString& compiler) const {
    return compilerCapabilities.contains(compiler);
}

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
        << "RayGeneration" << "Intersection" << "AnyHit" << "ClosestHit"
        << "Miss" << "Callable" << "Mesh";
    dxc.supportedShaderModels = QStringList() << "5_0" << "5_1" << "6_0" << "6_4";
    dxc.supportedOutputTypes = QStringList() << "DXIL" << "SPIR-V";
    compilerCapabilities["DXC"] = dxc;

    // GLSLANG 编译器配置
    CompilerCapability glslang;
    glslang.supportedShaderTypes = QStringList() 
        << "Vertex" << "Pixel" << "Geometry" << "Compute"
        << "RayGeneration" << "Intersection" << "AnyHit" << "ClosestHit"
        << "Miss" << "Callable" << "Mesh";
    glslang.supportedShaderModels = QStringList() << "450" << "460";
    glslang.supportedOutputTypes = QStringList() << "SPIR-V";
    compilerCapabilities["GLSLANG"] = glslang;

    // SPIRV-Cross 配置
    CompilerCapability spirvCross;
    spirvCross.supportedShaderTypes = QStringList() 
        << "Vertex" << "Pixel" << "Geometry" << "Compute";
    spirvCross.supportedShaderModels = QStringList();  // SPIRV-Cross 不需要 shader model
    spirvCross.supportedOutputTypes = QStringList() << "HLSL" << "GLSL";
    compilerCapabilities["SPIRV-CROSS"] = spirvCross;
}
