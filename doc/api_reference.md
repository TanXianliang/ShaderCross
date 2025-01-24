# API参考

## MainWindow 类
### 核心方法
```cpp
// 执行shader编译
void compileShader();
// 参数: 
//   - inputPath: 输入文件路径
//   - compiler: 选择的编译器类型
//   - entryPoint: 着色器入口函数
```

### 信号与槽
```cpp
// 编译完成信号
void compilationFinished(bool success, QString output);
// 参数:
//   - success: 编译是否成功
//   - output: 输出结果或错误信息

// 格式转换槽函数
void onOutputFormatChanged(QString newFormat);
```

### 工具函数
```cpp
// DXIL转SPIR-V实现
void convertDxilToSpirv(QString dxilPath);
// GLSL转DXIL实现  
void convertGlslToDxil(QString glslPath);
