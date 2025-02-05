#include "shaderCodeTextEdit.h"

QStringList GetKeyWords(const QString &language)
{
    if (language == "HLSL") {
        // 这里可以添加 HLSL 特有的关键字
        return QStringList() << 
            "bool" << "int" << "uint" << "float" << "half" << "double" <<
            "bool1" << "bool2" << "bool3" << "bool4" <<
            "int1" << "int2" << "int3" << "int4" <<
            "uint1" << "uint2" << "uint3" << "uint4" <<
            "float1" << "float2" << "float3" << "float4" <<
            "half1" << "half2" << "half3" << "half4" <<
            "double1" << "double2" << "double3" << "double4" <<
            "string" << "texture" << "texture1D" << "texture1DArray" <<
            "texture2D" << "texture2DArray" << "texture3D" << "textureCube" <<
            "textureCubeArray" << "RWTexture1D" << "RWTexture1DArray" <<
            "RWTexture2D" << "RWTexture2DArray" << "RWTexture3D" <<
            "StructuredBuffer" << "RWStructuredBuffer" << "ByteAddressBuffer" <<
            "RWByteAddressBuffer" << "Buffer" << "RWBuffer" <<

            // 结构体
            "struct" <<

            // 控制流
            "if" << "else" << "for" << "while" << "do" << "switch" << "case" << "default" <<
            "break" << "continue" << "return" << "discard" <<

            // 函数
            "main" << "sin" << "cos" << "tan" << "asin" << "acos" << "atan" <<
            "pow" << "exp" << "log" << "log10" << "sqrt" << "length" << "normalize" <<
            "dot" << "cross" << "reflect" << "refract" << "lerp" << "saturate" <<
            "clamp" << "step" << "smoothstep" << "fmod" << "ceil" << "floor" <<
            "round" << "frac" << "min" << "max" << "abs" << "sign" <<

            // 语义
            "in" << "out" << "inout" << "uniform" << "varying" << "attribute" << "const" <<
            "static" << "shared" << "group" << "register" << "location" << "binding" <<

            // 其他
            "texture2D" << "sampler" << "samplerState" << "samplerComparisonState" <<
            "Texture2D" << "Texture2DArray" << "Texture3D" << "TextureCube" <<
            "TextureCubeArray" << "RWTexture2D" << "RWTexture2DArray" <<
            "RWTexture3D" << "StructuredBuffer" << "RWStructuredBuffer" <<
            "ByteAddressBuffer" << "RWByteAddressBuffer" << "Buffer" << "RWBuffer";

    } else {
        // 默认使用 GLSL 关键字
        return QStringList() << 
            "void" << "bool" << "int" << "uint" << "float" << "double" <<
            "vec2" << "vec3" << "vec4" <<
            "bvec2" << "bvec3" << "bvec4" <<
            "ivec2" << "ivec3" << "ivec4" <<
            "uvec2" << "uvec3" << "uvec4" <<
            "mat2" << "mat3" << "mat4" <<
            "mat2x2" << "mat2x3" << "mat2x4" <<
            "mat3x2" << "mat3x3" << "mat3x4" <<
            "mat4x2" << "mat4x3" << "mat4x4" <<
            "sampler1D" << "sampler2D" << "sampler3D" << "samplerCube" <<
            "sampler1DArray" << "sampler2DArray" << "samplerCubeArray" <<
            "sampler2DShadow" << "samplerCubeShadow" <<
            "image1D" << "image2D" << "image3D" << "imageCube" <<
            "image1DArray" << "image2DArray" << "imageCubeArray" <<
            "samplerBuffer" << "imageBuffer" <<

            // 结构体
            "struct" <<

            // 控制流
            "if" << "else" << "for" << "while" << "do" << "switch" << "case" << "default" <<
            "break" << "continue" << "return" << "discard" <<

            // 函数
            "main" << "sin" << "cos" << "tan" << "asin" << "acos" << "atan" <<
            "pow" << "exp" << "log" << "log2" << "sqrt" << "length" << "normalize" <<
            "dot" << "cross" << "reflect" << "refract" << "mix" << "step" <<
            "smoothstep" << "clamp" << "min" << "max" << "abs" << "sign" <<

            // 语义
            "in" << "out" << "inout" << "uniform" << "varying" << "const" <<

            // 其他
            "texture" << "texture1D" << "texture2D" << "texture3D" << "textureCube" <<
            "texture1DArray" << "texture2DArray" << "textureCubeArray" <<
            "sampler" << "sampler1D" << "sampler2D" << "sampler3D" << "samplerCube" <<
            "sampler1DArray" << "sampler2DArray" << "samplerCubeArray" <<
            "layout" << "location" << "binding" << "shared" << "flat" << "centroid" <<
            "sample" << "patch" << "vertex" << "fragment" << "compute" << "geometry" <<
            "tessellation_control" << "tessellation_evaluation" << "raygeneration" <<
            "intersection" << "anyhit" << "closesthit" << "miss" << "callable" <<
            "mesh" << "task";
    }
}

ShaderCodeHighlighter::ShaderCodeHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    // 定义关键字
    keywords = GetKeyWords("HLSL");
}

void ShaderCodeHighlighter::highlightBlock(const QString &text) {
    highlightKeyword(text);
    highlightComment(text);
    highlightString(text);
}

void ShaderCodeHighlighter::highlightKeyword(const QString &text) {
    foreach (const QString &keyword, keywords) {
        QRegularExpression expression(QString("\\b%1\\b").arg(keyword));
        QRegularExpressionMatch match = expression.match(text);
        if (match.hasMatch()) {
            int index = match.capturedStart();
            setFormat(index, keyword.length(), QColor(173, 216, 230)); // 粉蓝色
        }
    }
}

void ShaderCodeHighlighter::highlightComment(const QString &text) {
    QRegularExpression commentExpression("//.*");
    QRegularExpressionMatchIterator it = commentExpression.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), Qt::green); // 注释高亮为绿色
    }
}

void ShaderCodeHighlighter::highlightString(const QString &text) {
    QRegularExpression stringExpression("\".*\"");
    QRegularExpressionMatchIterator it = stringExpression.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), Qt::darkRed); // 字符串高亮为深红色
    }
}

ShaderCodeTextEdit::ShaderCodeTextEdit(QWidget *parent)
    : QPlainTextEdit(parent), highlighter(new ShaderCodeHighlighter(document())) {
    // 设置默认的着色器语言
    setShaderLanguage("GLSL");
}

void ShaderCodeTextEdit::setShaderLanguage(const QString &language) {
    // 根据语言设置高亮规则（可以扩展）
    highlighter->keywords = GetKeyWords(language);
    highlighter->rehighlight(); // 重新高亮
}
