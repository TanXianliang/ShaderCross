#include "compilerSettingUI.h"
#include <QDebug>

CompilerSettingUI::CompilerSettingUI(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
}

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

    // 编译按钮
    buildButton = new QPushButton(tr("Build"), this);
    compilerLayout->addWidget(buildButton);

    mainLayout->addWidget(compilerGroup);
}

void CompilerSettingUI::setupConnections()
{
    connect(compilerCombo, &QComboBox::currentTextChanged, 
            this, &CompilerSettingUI::updateCompilerSettings);
    connect(compilerCombo, &QComboBox::currentTextChanged,
            this, &CompilerSettingUI::compilerChanged);
    connect(buildButton, &QPushButton::clicked, 
            this, &CompilerSettingUI::buildClicked);
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

void CompilerSettingUI::updateCompilerSettings(const QString &compiler)
{
    shaderTypeCombo->clear();
    shaderModelCombo->clear();
    outputTypeCombo->clear();

    if (CompilerConfig::instance().hasCompiler(compiler)) {
        const auto& capability = CompilerConfig::instance().getCapability(compiler);
        shaderTypeCombo->addItems(capability.supportedShaderTypes);
        shaderModelCombo->addItems(capability.supportedShaderModels);
        outputTypeCombo->addItems(capability.supportedOutputTypes);
    }
}

void CompilerSettingUI::onLanguageChanged(const QString &language)
{
    compilerCombo->clear();
    if (LanguageConfig::instance().hasLanguage(language)) {
        QStringList compilers = LanguageConfig::instance().getSupportedCompilers(language);
        compilerCombo->addItems(compilers);
    }
}
