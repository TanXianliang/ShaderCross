#include "compilerSettingUI.h"
#include <QDebug>

// 构造函数，初始化编译器设置 UI。
CompilerSettingUI::CompilerSettingUI(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
}

// 设置 UI 组件
void CompilerSettingUI::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGroupBox *compilerGroup = new QGroupBox(tr("Compiler Settings"), this);
    QVBoxLayout *compilerLayout = new QVBoxLayout(compilerGroup);

    // 编译器选择
    QHBoxLayout *compilerSelectLayout = new QHBoxLayout();
    compilerSelectLayout->addWidget(new QLabel(tr("Compiler:")));
    compilerCombo = new QComboBox(this);
    compilerSelectLayout->addWidget(compilerCombo);
    compilerLayout->addLayout(compilerSelectLayout);

    // Shader类型选择
    QHBoxLayout *shaderTypeLayout = new QHBoxLayout();
    shaderTypeLayout->addWidget(new QLabel(tr("Shader Type:")));
    shaderTypeCombo = new QComboBox(this);
    shaderTypeLayout->addWidget(shaderTypeCombo);
    compilerLayout->addLayout(shaderTypeLayout);

    // 入口点设置
    QHBoxLayout *entryPointLayout = new QHBoxLayout();
    entryPointLayout->addWidget(new QLabel(tr("Entry Point:")));
    entryPointEdit = new QLineEdit(this);
    entryPointEdit->setText("main");  // 默认入口点
    entryPointLayout->addWidget(entryPointEdit);
    compilerLayout->addLayout(entryPointLayout);

    // Shader Model 选择
    QHBoxLayout *shaderModelLayout = new QHBoxLayout();
    shaderModelLayout->addWidget(new QLabel(tr("Shader Model:")));
    shaderModelCombo = new QComboBox(this);
    shaderModelLayout->addWidget(shaderModelCombo);
    compilerLayout->addLayout(shaderModelLayout);

    // 输出类型选择
    QHBoxLayout *outputTypeLayout = new QHBoxLayout();
    outputTypeLayout->addWidget(new QLabel(tr("Output Type:")));
    outputTypeCombo = new QComboBox(this);
    outputTypeLayout->addWidget(outputTypeCombo);
    compilerLayout->addLayout(outputTypeLayout);

    // 在构建按钮之前添加额外编译选项控件
    QHBoxLayout *extraOptionsLayout = new QHBoxLayout();
    extraOptionsCheckBox = new QCheckBox(tr("Additional Options:"), this);
    extraOptionsCheckBox->setCheckState(Qt::Unchecked); // 默认未选中
    extraOptionsEdit = new QLineEdit(this);
    extraOptionsEdit->setEnabled(false); // 默认禁用
    
    extraOptionsLayout->addWidget(extraOptionsCheckBox);
    extraOptionsLayout->addWidget(extraOptionsEdit, 1); // 让输入框占据更多空间
    compilerLayout->addLayout(extraOptionsLayout);
    
    // 构建按钮
    buildButton = new QPushButton(tr("Build"), this);
    compilerLayout->addWidget(buildButton);

    mainLayout->addWidget(compilerGroup);
}

// 设置信号连接
void CompilerSettingUI::setupConnections()
{
    connect(compilerCombo, &QComboBox::currentTextChanged, 
            this, &CompilerSettingUI::updateCompilerSettings);
    connect(compilerCombo, &QComboBox::currentTextChanged,
            this, &CompilerSettingUI::compilerChanged);
    connect(buildButton, &QPushButton::clicked, 
            this, &CompilerSettingUI::buildClicked);
    
    // 连接额外选项复选框信号
    connect(extraOptionsCheckBox, &QCheckBox::toggled, extraOptionsEdit, &QLineEdit::setEnabled);
}

// 获取当前设置
QString CompilerSettingUI::getCurrentCompiler() const
{
    return compilerCombo->currentText();
}

QString CompilerSettingUI::getShaderType() const
{
    return shaderTypeCombo->currentText();
}

QString CompilerSettingUI::getEntryPoint() const
{
    return entryPointEdit->text();
}

QString CompilerSettingUI::getShaderModel() const
{
    return shaderModelCombo->currentText();
}

QString CompilerSettingUI::getOutputType() const
{
    return outputTypeCombo->currentText();
}

// 设置当前配置
void CompilerSettingUI::setCurrentCompiler(const QString &compiler)
{
    compilerCombo->setCurrentText(compiler);
}

void CompilerSettingUI::setShaderType(const QString &type)
{
    shaderTypeCombo->setCurrentText(type);
}

void CompilerSettingUI::setEntryPoint(const QString &entry)
{
    entryPointEdit->setText(entry);
}

void CompilerSettingUI::setShaderModel(const QString &model)
{
    shaderModelCombo->setCurrentText(model);
}

void CompilerSettingUI::setOutputType(const QString &type)
{
    outputTypeCombo->setCurrentText(type);
}

// 更新编译器设置
void CompilerSettingUI::updateCompilerSettings(const QString &compiler)
{
    QString currentShaderType = shaderTypeCombo->currentText();

    shaderTypeCombo->clear();
    shaderModelCombo->clear();
    outputTypeCombo->clear();

    if (CompilerConfig::instance().hasCompiler(compiler)) {
        const auto& capability = CompilerConfig::instance().getCapability(compiler);
        int currentIndex = 0;
        for (auto& iter : capability.supportedShaderTypes)
        {
            if (iter.contains(currentShaderType))
                break;

            currentIndex++;
        }
        shaderTypeCombo->addItems(capability.supportedShaderTypes);
        shaderTypeCombo->setCurrentIndex(currentIndex);
        
        shaderModelCombo->addItems(capability.supportedShaderModels);
        outputTypeCombo->addItems(capability.supportedOutputTypes);
    }
}

// 响应语言变化
void CompilerSettingUI::onLanguageChanged(const QString &language)
{
    if (LanguageConfig::instance().hasLanguage(language)) {
        QStringList compilers = LanguageConfig::instance().getSupportedCompilers(language);

        QString currentShaderType = shaderTypeCombo->currentText();
        QString currentCompiler = compilerCombo->currentText();

        compilerCombo->clear();
        compilerCombo->addItems(compilers);

        if (compilers.contains(currentCompiler))
            compilerCombo->setCurrentText(currentCompiler);
        else
            compilerCombo->setCurrentIndex(0);

        currentCompiler = compilerCombo->currentText();

        if (CompilerConfig::instance().hasCompiler(currentCompiler)) {
            const auto& capability = CompilerConfig::instance().getCapability(currentCompiler);

            if (capability.supportedShaderTypes.contains(currentShaderType))
                shaderTypeCombo->setCurrentText(currentShaderType);
            else
                shaderTypeCombo->setCurrentIndex(0);
        }
    }
}

// 实现新增的 getter 和 setter 方法
bool CompilerSettingUI::isExtraOptionsEnabled() const
{
    return extraOptionsCheckBox->isChecked();
}

QString CompilerSettingUI::getExtraOptions() const
{
    return extraOptionsEdit->text();
}

void CompilerSettingUI::setExtraOptionsEnabled(bool enabled)
{
    extraOptionsCheckBox->setChecked(enabled);
    extraOptionsEdit->setEnabled(enabled);
}

void CompilerSettingUI::setExtraOptions(const QString &options)
{
    extraOptionsEdit->setText(options);
}
