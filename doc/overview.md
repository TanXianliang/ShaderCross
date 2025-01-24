# Shader编译器程序文档

## 项目概述
本程序是一个基于Qt框架的Windows桌面应用程序，主要用于Shader代码的编译和格式转换。支持HLSL和GLSL输入，可使用多种编译器（DXC、FXC、GLSLANG）进行编译。

## 主要功能模块
- **输入处理模块**
  - HLSL/GLSL语言选择
  - 文件路径选择与验证
  - 文本编码支持（UTF-8、GB18030等）
  - 文本编辑器集成
  - 包含目录管理
  - 宏定义管理

- **编译器选项模块**
  - 多编译器支持（DXC/FXC/GLSLANG）
  - Shader类型选择
  - 入口点设置
  - Shader Model选择（5.0-6.4）

- **界面功能**
  - 深色/浅色主题切换
  - 布局重置
  - 输出面板显示/隐藏
  - 设置自动保存/加载

## 界面布局
程序界面分为左右两个主要区域：

### 左侧面板（Shader输入区）
1. **Shader Input**
   - Shader语言选择（HLSL/GLSL）
   - 文件路径设置
   - 文本编码选择
   - 代码编辑器

2. **Include Paths**
   - 包含路径列表
   - 添加/删除路径按钮

3. **Macro Definitions**
   - 宏定义列表
   - 添加/删除宏按钮

### 右侧面板（编译选项与输出区）
1. **Compiler Options**
   - 编译器选择（DXC/FXC/GLSLANG）
   - Shader类型选择
   - 入口点设置
   - Shader Model选择

2. **Output**
   - 编译输出显示
   - 错误信息显示

### 菜单栏
- **File**: 打开/保存文件
- **Edit**: 复制/粘贴
- **Build**: 编译/显示反汇编
- **UI**: 布局重置/主题切换/输出面板切换

## 技术特性
- 自动保存所有设置到config目录
- 记住上次打开文件的目录
- 支持HLSL/GLSL的编译器自动切换
- 防止重复添加包含路径和宏定义
- 完整的深色主题支持

## 技术栈
- Qt 5.15 (UI框架)
- CMake (构建系统)
- DXC编译器 (DirectX Shader Compiler)
- glslang (Vulkan GLSL编译器)
- SPIRV-Cross (着色器交叉编译)

## 项目结构
```
shader-compiler/
├── CMakeLists.txt        - 主构建配置
├── main.cpp              - 程序入口
├── mainwindow.[h/cpp]    - 主窗口实现
├── mainwindow.ui         - Qt界面设计文件
└── build/                - 构建输出目录
```

## 编译与运行
```bash
cmake -B build
cmake --build build --config Release
build/Release/ShaderCompiler.exe
