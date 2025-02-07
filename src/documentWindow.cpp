#include "documentWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QInputDialog>
#include <QCoreApplication>
#include "fxcCompiler.h"
#include "dxcCompiler.h"
#include "glslangCompiler.h"
#include "glslangkgverCompiler.h"
#include <QDialogButtonBox>

DocumentWindow::DocumentWindow(QWidget *parent, const QString &documentTitle)
    : QMainWindow(parent)
    , documentWindowTitle(documentTitle)
    , lastHLSLCompiler("DXC")
    , lastGLSLCompiler("GLSLANG")
    , lastOpenDir(QDir::currentPath())
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();
    setupConnections();
    loadSettings();
}

void DocumentWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 创建顶部布局
    QHBoxLayout *topLayout = new QHBoxLayout();

    // 左侧设置面板
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();
    
    // 语言选择
    QHBoxLayout *languageLayout = new QHBoxLayout();
    languageLayout->addWidget(new QLabel(tr("Language:")));
    languageCombo = new QComboBox(this);
    languageCombo->addItems({"HLSL", "GLSL"});
    languageLayout->addWidget(languageCombo);
    leftPanelLayout->addLayout(languageLayout);

    // 编码选择
    QHBoxLayout *encodingLayout = new QHBoxLayout();
    encodingLayout->addWidget(new QLabel(tr("Encoding:")));
    encodingCombo = new QComboBox(this);
    encodingCombo->addItems({"UTF-8", "GB2312", "GBK"});
    encodingLayout->addWidget(encodingCombo);
    leftPanelLayout->addLayout(encodingLayout);

    // 编译器设置
    compilerSettingUI = new CompilerSettingUI(this);
    leftPanelLayout->addWidget(compilerSettingUI);

    // 包含路径
    QGroupBox *includeGroup = new QGroupBox(tr("Include Paths"), this);
    QVBoxLayout *includeLayout = new QVBoxLayout(includeGroup);
    includePathList = new QListWidget(this);
    includeLayout->addWidget(includePathList);
    
    QHBoxLayout *includeButtonLayout = new QHBoxLayout();
    addIncludeButton = new QPushButton(tr("Add"), this);
    removeIncludeButton = new QPushButton(tr("Remove"), this);
    includeButtonLayout->addWidget(addIncludeButton);
    includeButtonLayout->addWidget(removeIncludeButton);
    includeLayout->addLayout(includeButtonLayout);
    leftPanelLayout->addWidget(includeGroup);

    // 宏定义
    QGroupBox *macroGroup = new QGroupBox(tr("Macros"), this);
    QVBoxLayout *macroLayout = new QVBoxLayout(macroGroup);
    macroList = new QListWidget(this);
    macroLayout->addWidget(macroList);
    
    QHBoxLayout *macroButtonLayout = new QHBoxLayout();
    addMacroButton = new QPushButton(tr("Add"), this);
    removeMacroButton = new QPushButton(tr("Remove"), this);
    macroButtonLayout->addWidget(addMacroButton);
    macroButtonLayout->addWidget(removeMacroButton);
    macroLayout->addLayout(macroButtonLayout);
    leftPanelLayout->addWidget(macroGroup);

    // 编译按钮
    compileButton = new QPushButton(tr("Compile"), this);
    leftPanelLayout->addWidget(compileButton);

    // 添加左侧面板到顶部布局
    topLayout->addLayout(leftPanelLayout);

    // 创建右侧编辑器和输出区域
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();
    
    // 文件路径输入区域
    QGroupBox *inputGroup = new QGroupBox(tr("Input"), this);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    
    QHBoxLayout *filePathLayout = new QHBoxLayout();
    filePathEdit = new QLineEdit(this);
    filePathEdit->setReadOnly(true);
    browseButton = new QPushButton(tr("Browse"), this);
    filePathLayout->addWidget(filePathEdit);
    filePathLayout->addWidget(browseButton);
    inputLayout->addLayout(filePathLayout);

    // 输入编辑器
    inputEdit = new ShaderCodeTextEdit(this);
    inputLayout->addWidget(inputEdit);
    rightPanelLayout->addWidget(inputGroup);

    // 输出和日志
    QSplitter *outputSplitter = new QSplitter(Qt::Horizontal, this);
    
    QGroupBox *outputGroup = new QGroupBox(tr("Output"), this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    outputEdit = new QTextEdit(this);
    outputLayout->addWidget(outputEdit);
    outputSplitter->addWidget(outputGroup);
    
    QGroupBox *logGroup = new QGroupBox(tr("Log"), this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    logEdit = new QTextEdit(this);
    logLayout->addWidget(logEdit);
    outputSplitter->addWidget(logGroup);

    rightPanelLayout->addWidget(outputSplitter);

    // 添加右侧面板到顶部布局
    topLayout->addLayout(rightPanelLayout, 1);

    // 添加顶部布局到主布局
    mainLayout->addLayout(topLayout);

    setCentralWidget(centralWidget);
    resize(1200, 800);
}

void DocumentWindow::setupConnections()
{
    // 编译按钮
    connect(compileButton, &QPushButton::clicked, this, &DocumentWindow::compile);

    // 包含路径按钮
    connect(addIncludeButton, &QPushButton::clicked, this, &DocumentWindow::addIncludePath);
    connect(removeIncludeButton, &QPushButton::clicked, this, &DocumentWindow::removeIncludePath);
    connect(includePathList, &QListWidget::itemSelectionChanged, this, [this]() {
        removeIncludeButton->setEnabled(!includePathList->selectedItems().isEmpty());
    });

    // 宏定义按钮
    connect(addMacroButton, &QPushButton::clicked, this, &DocumentWindow::addMacro);
    connect(removeMacroButton, &QPushButton::clicked, this, &DocumentWindow::removeMacro);
    connect(macroList, &QListWidget::itemSelectionChanged, this, [this]() {
        removeMacroButton->setEnabled(!macroList->selectedItems().isEmpty());
    });

    // 语言切换
    connect(languageCombo, &QComboBox::currentTextChanged, this, [this](const QString &language) {
        compilerSettingUI->onLanguageChanged(language);
        inputEdit->setShaderLanguage(language);
    });

    // 编译器设置变更
    connect(compilerSettingUI, &CompilerSettingUI::compilerChanged, 
            this, &DocumentWindow::updateCurrentCompilerSettings);

    // 文件浏览按钮连接
    connect(browseButton, &QPushButton::clicked, this, &DocumentWindow::onBrowseFile);
    connect(encodingCombo, &QComboBox::currentTextChanged, [this]() {
        if (!filePathEdit->text().isEmpty()) {
            loadFileContent(filePathEdit->text());
        }
    });
}

void DocumentWindow::compile()
{
    QString compiler = compilerSettingUI->getCurrentCompiler();
    QString shaderType = compilerSettingUI->getShaderType();
    QString shaderModel = compilerSettingUI->getShaderModel();
    QString entryPoint = compilerSettingUI->getEntryPoint();
    QString outputType = compilerSettingUI->getOutputType();

    // 获取包含路径和宏定义
    QStringList includePaths;
    for (int i = 0; i < includePathList->count(); ++i) {
        includePaths << includePathList->item(i)->text();
    }

    QStringList macros;
    for (int i = 0; i < macroList->count(); ++i) {
        macros << macroList->item(i)->text();
    }

    // 根据选择的编译器创建相应的实例
    if (compiler == "FXC") {
        fxcCompiler *fxcCompilerInstance = new fxcCompiler(this);
        
        connect(fxcCompilerInstance, &fxcCompiler::compilationFinished, this, [this](const QString &output) {
            outputEdit->setTextColor(Qt::green);
            outputEdit->append(tr("Compilation succeeded:\n") + output);
            logEdit->setTextColor(Qt::green);
            logEdit->append(tr("Compilation succeeded"));
        });

        connect(fxcCompilerInstance, &fxcCompiler::compilationError, this, [this](const QString &error) {
            outputEdit->setTextColor(Qt::red);
            outputEdit->append(tr("Compilation error:\n") + error);
            logEdit->setTextColor(Qt::red);
            logEdit->append(tr("Compilation failed"));
        });

        connect(fxcCompilerInstance, &fxcCompiler::compilationWarning, this, [this](const QString &warning) {
            outputEdit->setTextColor(Qt::yellow);
            outputEdit->append(tr("Compilation warning:\n") + warning);
            logEdit->setTextColor(Qt::yellow);
            logEdit->append(tr("Compilation warning"));
        });

        fxcCompilerInstance->compile(inputEdit->toPlainText(), shaderModel, entryPoint, shaderType, includePaths, macros);
    } else if (compiler == "DXC") {
        dxcCompiler *dxcCompilerInstance = new dxcCompiler(this);
        
        connect(dxcCompilerInstance, &dxcCompiler::compilationFinished, this, [this](const QString &output) {
            outputEdit->setTextColor(Qt::green);
            outputEdit->append(tr("Compilation succeeded:\n") + output);
            logEdit->setTextColor(Qt::green);
            logEdit->append(tr("Compilation succeeded"));
        });

        connect(dxcCompilerInstance, &dxcCompiler::compilationError, this, [this](const QString &error) {
            outputEdit->setTextColor(Qt::red);
            outputEdit->append(tr("Compilation error:\n") + error);
            logEdit->setTextColor(Qt::red);
            logEdit->append(tr("Compilation failed"));
        });

        connect(dxcCompilerInstance, &dxcCompiler::compilationWarning, this, [this](const QString &warning) {
            outputEdit->setTextColor(Qt::yellow);
            outputEdit->append(tr("Compilation warning:\n") + warning);
            logEdit->setTextColor(Qt::yellow);
            logEdit->append(tr("Compilation warning"));
        });

        dxcCompilerInstance->compile(inputEdit->toPlainText(), shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    } else if (compiler == "GLSLANG") {
        glslangCompiler *glslangCompilerInstance = new glslangCompiler(this);
        
        connect(glslangCompilerInstance, &glslangCompiler::compilationFinished, this, [this](const QString &output) {
            outputEdit->setTextColor(Qt::green);
            outputEdit->append(tr("Compilation succeeded:\n") + output);
            logEdit->setTextColor(Qt::green);
            logEdit->append(tr("Compilation succeeded"));
        });

        connect(glslangCompilerInstance, &glslangCompiler::compilationError, this, [this](const QString &error) {
            outputEdit->setTextColor(Qt::red);
            outputEdit->append(tr("Compilation error:\n") + error);
            logEdit->setTextColor(Qt::red);
            logEdit->append(tr("Compilation failed"));
        });

        connect(glslangCompilerInstance, &glslangCompiler::compilationWarning, this, [this](const QString &warning) {
            outputEdit->setTextColor(Qt::yellow);
            outputEdit->append(tr("Compilation warning:\n") + warning);
            logEdit->setTextColor(Qt::yellow);
            logEdit->append(tr("Compilation warning"));
        });

        glslangCompilerInstance->compile(inputEdit->toPlainText(), languageCombo->currentText(), shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    } else if (compiler == "GLSLANGKGVER") {
        glslangkgverCompiler *glslangkgverCompilerInstance = new glslangkgverCompiler(this);
        
        connect(glslangkgverCompilerInstance, &glslangkgverCompiler::compilationFinished, this, [this](const QString &output) {
            outputEdit->setTextColor(Qt::green);
            outputEdit->append(tr("Compilation succeeded:\n") + output);
            logEdit->setTextColor(Qt::green);
            logEdit->append(tr("Compilation succeeded"));
        });

        connect(glslangkgverCompilerInstance, &glslangkgverCompiler::compilationError, this, [this](const QString &error) {
            outputEdit->setTextColor(Qt::red);
            outputEdit->append(tr("Compilation error:\n") + error);
            logEdit->setTextColor(Qt::red);
            logEdit->append(tr("Compilation failed"));
        });

        glslangkgverCompilerInstance->compile(inputEdit->toPlainText(), shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    }
}

void DocumentWindow::addIncludePath()
{
    QString dir = QFileDialog::getExistingDirectory(this,
        tr("Select Include Directory"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        
    if (!dir.isEmpty()) {
        // 检查是否已存在该路径
        QList<QListWidgetItem*> items = includePathList->findItems(dir, Qt::MatchExactly);
        if (items.isEmpty()) {
            includePathList->addItem(dir);
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("This path already exists in the include list."));
        }
    }
}

void DocumentWindow::removeIncludePath()
{
    QList<QListWidgetItem*> items = includePathList->selectedItems();
    for (QListWidgetItem *item : items) {
        delete includePathList->takeItem(includePathList->row(item));
    }
    updateIncludeListHeight();
}

void DocumentWindow::addMacro()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Add Macro Definition"));
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    // 宏名称输入
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(tr("Name:"), &dialog);
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);
    layout->addLayout(nameLayout);
    
    // 宏值输入
    QHBoxLayout *valueLayout = new QHBoxLayout();
    QLabel *valueLabel = new QLabel(tr("Value:"), &dialog);
    QLineEdit *valueEdit = new QLineEdit(&dialog);
    valueLayout->addWidget(valueLabel);
    valueLayout->addWidget(valueEdit);
    layout->addLayout(valueLayout);
    
    // 确定取消按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        QString value = valueEdit->text().trimmed();
        if (!name.isEmpty()) {
            // 检查是否已存在同名宏
            bool exists = false;
            for (int i = 0; i < macroList->count(); ++i) {
                QString existingMacro = macroList->item(i)->text();
                QString existingName = existingMacro.split('=').first().trimmed();
                if (existingName == name) {
                    exists = true;
                    break;
                }
            }
            
            if (!exists) {
                QString macro = name;
                if (!value.isEmpty()) {
                    macro += "=" + value;
                }
                macroList->addItem(macro);
                updateMacroListHeight();
            } else {
                QMessageBox::warning(this, tr("Warning"), tr("A macro with this name already exists."));
            }
        }
    }
}

void DocumentWindow::removeMacro()
{
    QList<QListWidgetItem*> items = macroList->selectedItems();
    for (QListWidgetItem *item : items) {
        delete macroList->takeItem(macroList->row(item));
    }
    updateMacroListHeight();
}

void DocumentWindow::updateIncludeListHeight()
{
    const int rowHeight = 24;  // 每行的高度
    const int minRows = 2;     // 最少显示行数
    const int maxRows = 4;     // 最多显示行数
    
    int itemCount = includePathList->count();
    int neededRows = qBound(minRows, itemCount, maxRows);
    
    int newHeight = neededRows * rowHeight + 8;  // 8px for padding
    includePathList->setFixedHeight(newHeight);
}

void DocumentWindow::updateMacroListHeight()
{
    const int rowHeight = 24;  // 每行的高度
    const int minRows = 2;     // 最少显示行数
    const int maxRows = 4;     // 最多显示行数
    
    int itemCount = macroList->count();
    int neededRows = qBound(minRows, itemCount, maxRows);
    
    int newHeight = neededRows * rowHeight + 8;  // 8px for padding
    macroList->setFixedHeight(newHeight);
}

void DocumentWindow::updateCurrentCompilerSettings(const QString &compiler)
{
    if (languageCombo->currentText() == "HLSL") {
        lastHLSLCompiler = compiler;
    } else {
        lastGLSLCompiler = compiler;
    }
}

QString DocumentWindow::settingsFilePath() const
{
    // 使用程序所在目录下的config目录
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + "/config";
    QDir().mkpath(configPath);  // 确保config目录存在
    return configPath + "/" + documentWindowTitle + "_settings.ini";  // 返回完整的配置文件路径
}

void DocumentWindow::loadSettings()
{
    QString settingsPath = settingsFilePath();
    QSettings settings(settingsPath, QSettings::IniFormat);
    
    // 恢复语言设置
    QString language = settings.value("language", "HLSL").toString();
    languageCombo->setCurrentText(language);
    compilerSettingUI->onLanguageChanged(language);  // 更新编译器设置

    // 恢复编译器设置
    QString compiler = settings.value("compiler").toString();
    if (!compiler.isEmpty()) {
        compilerSettingUI->setCurrentCompiler(compiler);
    }
    
    QString entryPoint = settings.value("entryPoint", "main").toString();
    compilerSettingUI->setEntryPoint(entryPoint);
    
    QString shaderType = settings.value("shaderType", "Vertex").toString();
    compilerSettingUI->setShaderType(shaderType);
    
    QString shaderModel = settings.value("shaderModel", "5.0").toString();
    compilerSettingUI->setShaderModel(shaderModel);
    
    QString outputType = settings.value("outputType", "DXIL").toString();
    compilerSettingUI->setOutputType(outputType);
    
    lastOpenDir = settings.value("lastOpenDir", QDir::currentPath()).toString();
    
    // 恢复编码
    encodingCombo->setCurrentText(settings.value("encoding", "UTF-8").toString());
    
    // 恢复包含路径
    QStringList includePaths = settings.value("includePaths").toStringList();
    includePathList->clear();
    includePathList->addItems(includePaths);
    
    // 恢复宏定义
    QStringList macros = settings.value("macros").toStringList();
    macroList->clear();
    macroList->addItems(macros);

    // 恢复输入框内容
    inputEdit->setPlainText(settings.value("inputContent").toString());
}

void DocumentWindow::saveSettings()
{
    QString settingsPath = settingsFilePath();
    QSettings settings(settingsPath, QSettings::IniFormat);
    
    // 保存语言设置
    settings.setValue("language", languageCombo->currentText());
    
    // 保存编译器设置
    settings.setValue("compiler", compilerSettingUI->getCurrentCompiler());
    settings.setValue("entryPoint", compilerSettingUI->getEntryPoint());
    settings.setValue("shaderType", compilerSettingUI->getShaderType());
    settings.setValue("shaderModel", compilerSettingUI->getShaderModel());
    settings.setValue("outputType", compilerSettingUI->getOutputType());
    
    // 保存编码
    settings.setValue("encoding", encodingCombo->currentText());
    
    // 保存包含路径
    QStringList includePaths;
    for (int i = 0; i < includePathList->count(); ++i) {
        includePaths << includePathList->item(i)->text();
    }
    settings.setValue("includePaths", includePaths);
    
    // 保存宏定义
    QStringList macros;
    for (int i = 0; i < macroList->count(); ++i) {
        macros << macroList->item(i)->text();
    }
    settings.setValue("macros", macros);
    
    // 保存编译器历史记录
    settings.setValue("lastHLSLCompiler", lastHLSLCompiler);
    settings.setValue("lastGLSLCompiler", lastGLSLCompiler);
    
    // 保存最后打开的目录
    settings.setValue("lastOpenDir", lastOpenDir);
    
    // 保存输入框内容
    settings.setValue("inputContent", inputEdit->toPlainText());
}

void DocumentWindow::onBrowseFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Shader File"),
        lastOpenDir,
        tr("Shader Files (*.hlsl *.glsl *.vert *.frag *.comp);;All Files (*.*)")
    );

    if (!filePath.isEmpty()) {
        lastOpenDir = QFileInfo(filePath).absolutePath();
        filePathEdit->setText(filePath);
        loadFileContent(filePath);
    }
}

void DocumentWindow::loadFileContent(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
                           tr("Cannot read file %1:\n%2.")
                           .arg(filePath)
                           .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    // 设置文件编码
    if (encodingCombo->currentText() == "UTF-8") {
        in.setCodec("UTF-8");
    } else if (encodingCombo->currentText() == "GB2312") {
        in.setCodec("GB2312");
    } else if (encodingCombo->currentText() == "GBK") {
        in.setCodec("GBK");
    }

    QString content = in.readAll();
    file.close();

    inputEdit->setPlainText(content);
    
    // 根据文件扩展名自动设置语言
    QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == "hlsl") {
        languageCombo->setCurrentText("HLSL");
    } else if (suffix == "glsl" || suffix == "vert" || suffix == "frag" || suffix == "comp") {
        languageCombo->setCurrentText("GLSL");
    }
}

DocumentWindow::~DocumentWindow()
{
    saveSettings();
}

