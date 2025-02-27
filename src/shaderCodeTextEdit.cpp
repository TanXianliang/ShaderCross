#include "shaderCodeTextEdit.h"

QStringList GetKeyWords(const QString &language)
{
    if (language == "HLSL") {
        QStringList hlslBuiltInVariables = QStringList() <<

        // 顶点着色器（Vertex Shader）中的内置变量
        "SV_VertexID" << "SV_InstanceID" << "SV_Position" <<

        // 片段着色器（Pixel Shader）中的内置变量
        "SV_Position" << "SV_IsFrontFace" << "SV_SampleIndex" << "SV_Coverage" << "SV_Targe" << "SV_Coverage" << 
        "SV_Target0" << "SV_Target1" << "SV_Target2" << "SV_Target3" << "SV_Target4" << "SV_Target5" << "SV_Target6" << "SV_Target7" << "SV_Target8" <<

        // 几何着色器（Geometry Shader）中的内置变量
        "SV_PrimitiveID" << "SV_GSInstanceID" <<

        // 计算着色器（Compute Shader）中的内置变量
        "SV_DispatchThreadID" << "SV_GroupID" << "SV_GroupIndex" << "SV_GroupThreadID";

        return QStringList() << hlslBuiltInVariables <<
            "bool" << "int" << "uint" << "float" << "half" << "double" <<
            "bool1" << "bool2" << "bool3" << "bool4" <<
            "int1" << "int2" << "int3" << "int4" <<
            "uint1" << "uint2" << "uint3" << "uint4" <<
            "float1" << "float2" << "float3" << "float4" <<
            "half1" << "half2" << "half3" << "half4" <<
            "double1" << "double2" << "double3" << "double4" <<
            "float2x2" << "float2x3" << "float2x4" <<
            "float3x2" << "float3x3" << "float3x4" <<
            "float4x2" << "float4x3" << "float4x4" <<
            "string" <<
            "SamplerState" << "Texture1D" << "Texture1DArray" <<
            "Texture2D" << "Texture2DArray" << "Texture3D" << "TextureCube" <<
            "TextureCubeArray" << "RWTexture1D" << "RWTexture1DArray" <<
            "RWTexture2D" << "RWTexture2DArray" << "RWTexture3D" <<
            "StructuredBuffer" << "RWStructuredBuffer" << "ByteAddressBuffer" <<
            "RWByteAddressBuffer" << "Buffer" << "RWBuffer" << "cbuffer" <<

            // 结构体
            "struct" << "packoffset" <<

            // 控制流
            "if" << "else" << "for" << "while" << "do" << "switch" << "case" << "default" <<
            "break" << "continue" << "return" << "discard" << "clip" <<

            // 函数
            "sin" << "cos" << "tan" << "asin" << "acos" << "atan" <<
            "pow" << "exp" << "log" << "log10" << "sqrt" << "length" << "normalize" <<
            "dot" << "cross" << "reflect" << "refract" << "lerp" << "saturate" <<
            "clamp" << "step" << "smoothstep" << "fmod" << "ceil" << "floor" <<
            "round" << "frac" << "min" << "max" << "abs" << "sign" << "any" << "all" <<
            "mul" <<

            // 语义
            "in" << "out" << "inout" << "uniform" << "varying" << "attribute" << "const" <<
            "static" << "groupshared" << "register" << "location" << "binding" << "numthreads" << "row_major" << "col_major" << "void"

            // 其他
            "StructuredBuffer" << "RWStructuredBuffer" <<
            "ByteAddressBuffer" << "RWByteAddressBuffer" << "Buffer" << "RWBuffer";

    } else {
        QStringList glslBuiltInVariables = QStringList() <<
        // 顶点着色器（Vertex Shader）中的内置变量
        "gl_VertexID" << "gl_VertexIndex" << "gl_InstanceID" << "gl_DrawID" << "gl_Position" << "gl_PointSize" << "gl_ClipDistance" <<

        // 片段着色器（Fragment Shader）中的内置变量
        "gl_FragCoord" << "gl_FrontFacing" << "gl_PointCoord" << "gl_SampleID" << "gl_SamplePosition" << "gl_SampleMaskIn" << "gl_FragDepth" << 
        "gl_FragColor" << // 已废弃
        "gl_SampleMask" <<

        // 几何着色器（Geometry Shader）中的内置变量
        "gl_PrimitiveIDIn" << "gl_InvocationID" << "gl_PrimitiveID" << "gl_Layer" << "gl_ViewportIndex" <<

        // 计算着色器（Compute Shader）中的内置变量
        "gl_NumWorkGroups" << "gl_WorkGroupID" << "gl_LocalInvocationID" << "gl_GlobalInvocationID" << "gl_LocalInvocationIndex" << 

        // 其他通用内置变量
        "gl_Position" << "gl_FragCoord" <<  "gl_PointSize" << "gl_FragDepth"; 

        // 默认使用 GLSL 关键字
        return QStringList() << glslBuiltInVariables <<
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
    highlightString(text);
    highlightComment(text);
}

void ShaderCodeHighlighter::highlightKeyword(const QString &text) {
    foreach (const QString &keyword, keywords) {
        QRegularExpression expression(QString("\\b%1\\b").arg(keyword));
        QRegularExpressionMatch match = expression.match(text);
        if (match.hasMatch()) {
            int index = match.capturedStart();
            setFormat(index, keyword.length(), QColor(152, 193, 252));
        }
    }
}

void ShaderCodeHighlighter::highlightComment(const QString &text) {
    QRegularExpression commentExpression("//.*");
    QRegularExpressionMatchIterator it = commentExpression.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), QColor(87, 166, 74)); 
    }
}

void ShaderCodeHighlighter::highlightString(const QString &text) {
    QRegularExpression stringExpression("\".*\"");
    QRegularExpressionMatchIterator it = stringExpression.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), QColor(227, 179, 104)); 
    }
}

ShaderCodeTextEdit::ShaderCodeTextEdit(QWidget *parent)
    : QPlainTextEdit(parent), highlighter(new ShaderCodeHighlighter(document())) {
    // 设置默认的着色器语言
    setShaderLanguage("HLSL");

    // 创建一个 QFont 对象并设置字体大小
    QFont fontInstance = font(); // 获取当前字体
    fontInstance.setPointSize(10); // 设置字体大小为 10
    setFont(fontInstance); // 应用新的字体
    
    // 设置Tab停止距离为4个空格的宽度
    int tabWidth = QFontMetrics(font()).horizontalAdvance("    "); // 计算 4 个空格的宽度
    setTabStopDistance(tabWidth);

    lineNumberArea = new LineNumberArea(this);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &ShaderCodeTextEdit::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &ShaderCodeTextEdit::updateLineNumberArea);
    updateLineNumberAreaWidth(0);
}

void ShaderCodeTextEdit::setShaderLanguage(const QString &language) {
    // 根据语言设置高亮规则（可以扩展）
    highlighter->keywords = GetKeyWords(language);
    highlighter->rehighlight(); // 重新高亮
}

int ShaderCodeTextEdit::lineNumberAreaWidth() {
    int digits = 1;
    int maxLines = qMax(1, blockCount());
    while (maxLines >= 10) {
        maxLines /= 10;
        ++digits;
    }
    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void ShaderCodeTextEdit::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

inline void ShaderCodeTextEdit::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(lineNumberArea);

    QFont fontInstance = font(); // 获取当前字体
    fontInstance.setPointSize(fontInstance.pointSize() - 1);
    painter.setFont(fontInstance); // 应用新的字体

    painter.fillRect(event->rect(), Qt::darkGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    auto boundingGeo = blockBoundingGeometry(block);
    qreal top = boundingGeo.top(); // 获取当前块的顶部位置
    qreal bottom = boundingGeo.bottom();

    while (block.isValid() && (int)top <= lineNumberArea->height()) {
        if (block.isVisible()) {
            painter.drawText(-2, top, lineNumberArea->width(), boundingGeo.height(),
                Qt::AlignRight, QString::number(blockNumber + 1));
        }
        block = block.next();
        boundingGeo = blockBoundingGeometry(block);
        top = boundingGeo.top(); // 获取当前块的顶部位置
        bottom = boundingGeo.bottom();
        ++blockNumber;
    }
}

LineNumberArea::LineNumberArea(ShaderCodeTextEdit* editor) : QWidget(editor) {
    this->editor = editor;
}

QSize LineNumberArea::sizeHint() const {
    return QSize(editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event) {
    editor->lineNumberAreaPaintEvent(event);
}
