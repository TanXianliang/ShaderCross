# Shader Compiler

一个跨平台的Shader编译工具，支持HLSL和GLSL的编译与转换。

## 功能特点

- 支持HLSL和GLSL输入
- 多编译器支持（DXC、FXC、GLSLANG）
- 完整的编译选项配置
- 包含路径和宏定义管理
- 深色/浅色主题支持
- 设置自动保存/加载

## 构建要求

- Qt 5.15+
- CMake 3.15+
- C++17 兼容的编译器

## 构建步骤

```bash
# 克隆仓库
git clone https://github.com/yourusername/shader-compiler.git
cd shader-compiler

# 创建构建目录
cmake -B build
cmake --build build --config Release
```

## 使用说明

详细的使用说明请参考：
- [项目概述](doc/overview.md)
- [用户指南](doc/user_guide.md)

## 许可证

本项目采用 MIT 许可证 