#ifndef COMPILERSETTINGUI_H
#define COMPILERSETTINGUI_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include "compilerConfig.h"
#include "languageConfig.h"
#include <QtWidgets/QCheckBox>

// CompilerSettingUI 类用于管理编译器设置的用户界面。
class CompilerSettingUI : public QWidget
{
    Q_OBJECT

public:
    // 构造函数，初始化编译器设置 UI。
    explicit CompilerSettingUI(QWidget *parent = nullptr);

    // 获取当前设置
    QString getCurrentCompiler() const;
    QString getShaderType() const;
    QString getEntryPoint() const;
    QString getShaderModel() const;
    QString getOutputType() const;
    bool isExtraOptionsEnabled() const; // 获取额外选项是否启用
    QString getExtraOptions() const; // 获取额外编译选项
    
    // 设置当前配置
    void setCurrentCompiler(const QString &compiler);
    void setShaderType(const QString &type);
    void setEntryPoint(const QString &entry);
    void setShaderModel(const QString &model);
    void setOutputType(const QString &type);
    void setExtraOptionsEnabled(bool enabled); // 设置额外选项是否启用
    void setExtraOptions(const QString &options); // 设置额外编译选项

public slots:
    // 响应语言变化
    void onLanguageChanged(const QString &language);
    
    // 更新编译器设置
    void updateCompilerSettings(const QString &compiler);

signals:
    void buildClicked(); // 构建按钮点击信号
    void compilerChanged(const QString &compiler); // 编译器变化信号

private:
    // UI 组件
    QComboBox *compilerCombo; // 编译器选择下拉框
    QComboBox *shaderTypeCombo; // 着色器类型选择下拉框
    QLineEdit *entryPointEdit; // 入口点输入框
    QComboBox *shaderModelCombo; // 着色器模型选择下拉框
    QComboBox *outputTypeCombo; // 输出类型选择下拉框
    QCheckBox *extraOptionsCheckBox; // 额外编译选项复选框
    QLineEdit *extraOptionsEdit; // 额外编译选项输入框
    QPushButton *buildButton; // 构建按钮

    // 设置 UI 组件
    void setupUI();
    
    // 设置信号连接
    void setupConnections();
};

#endif // COMPILERSETTINGUI_H
