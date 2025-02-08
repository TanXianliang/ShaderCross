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
#include <QDateTime>

DocumentWindow::DocumentWindow(QWidget *parent, const QString &documentTitle)
    : QMainWindow(parent)
    , documentWindowTitle(documentTitle)
    , lastHLSLCompiler("DXC")
    , lastGLSLCompiler("GLSLANG")
    , lastOpenDir(QDir::currentPath())
    , isSaveSettings(true)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();
    setupConnections();
    loadSettings();
}

 // Start of Selection
DocumentWindow::~DocumentWindow()
{
    if (isSaveSettings) {
        saveSettings(); 
    } else {
        // 删除settings文件
        QString settingsPath = settingsFilePath();
        QFile::remove(settingsPath);
    }

    // 销毁所有UI控件
    if (filePathEdit) {
        delete filePathEdit;
        filePathEdit = nullptr;
    }
    if (browseButton) {
        delete browseButton;
        browseButton = nullptr;
    }   
    if (languageCombo) {
        delete languageCombo;
        languageCombo = nullptr;
    }
    if (encodingCombo) {
        delete encodingCombo;
        encodingCombo = nullptr;
    }
    if (inputEdit) {
        delete inputEdit;
        inputEdit = nullptr;
    }
    if (includePathList) {
        delete includePathList;
        includePathList = nullptr;
    }
    if (addIncludeButton) {
        delete addIncludeButton;
        addIncludeButton = nullptr;
    }
    if (removeIncludeButton) {
        delete removeIncludeButton;
        removeIncludeButton = nullptr;
    }
    if (macroList) {
        delete macroList;
        macroList = nullptr;
    }
    if (addMacroButton) {
        delete addMacroButton;
        addMacroButton = nullptr;
    }
    if (removeMacroButton) {
        delete removeMacroButton;
        removeMacroButton = nullptr;
    }
    if (outputEdit) {
        delete outputEdit;
        outputEdit = nullptr;
    }
    if (logEdit) {
        delete logEdit;
        logEdit = nullptr;
    }
    if (compilerSettingUI) {
        delete compilerSettingUI;
        compilerSettingUI = nullptr;
    }
}

void DocumentWindow::enableSave(bool enable)
{
    isSaveSettings = enable;
}

void DocumentWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);  // 恢复间距
    mainLayout->setContentsMargins(12, 12, 12, 12);  // 恢复边距

    // 原有的界面布局
    QWidget *contentWidget = new QWidget(this);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);
    mainLayout->addWidget(contentWidget);

    // Left panel
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(12);  // 增加垂直间距
    
    // Input settings area
    QGroupBox *inputGroup = new QGroupBox(tr("Shader Input"), this);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(8);
    
    // Language selection
    QHBoxLayout *langLayout = new QHBoxLayout();
    languageCombo = new QComboBox(this);

    QStringList languages = LanguageConfig::instance().getSupportedLanguages();
    languageCombo->addItems(languages);
    langLayout->addWidget(new QLabel(tr("Shader Language:")));
    langLayout->addWidget(languageCombo);
    inputLayout->addLayout(langLayout);
    
    // File selection
    QHBoxLayout *fileLayout = new QHBoxLayout();
    filePathEdit = new QLineEdit(this);
    browseButton = new QPushButton(QIcon(), tr("Browse"), this);
    browseButton->setObjectName("browseButton");  // 添加对象名
    fileLayout->addWidget(new QLabel(tr("Shader Path:")));
    fileLayout->addWidget(filePathEdit);
    fileLayout->addWidget(browseButton);
    inputLayout->addLayout(fileLayout);

    // 添加编码选择
    QHBoxLayout *encodingLayout = new QHBoxLayout();
    encodingLayout->addWidget(new QLabel(tr("Encoding:")));
    encodingCombo = new QComboBox(this);
    encodingCombo->addItems(QStringList() << "UTF-8" << "GB18030" << "UTF-16" << "System");
    encodingCombo->setCurrentText("UTF-8");
    encodingLayout->addWidget(encodingCombo);
    inputLayout->addLayout(encodingLayout);

    // 添加文件内容显示区域
    inputEdit = new ShaderCodeTextEdit(this);
    inputEdit->setReadOnly(false);
    inputLayout->addWidget(inputEdit, 1);  // 添加拉伸因子1
    
    // 添加包含路径设置
    QGroupBox *includeGroup = new QGroupBox(tr("Include Paths"), this);
    QVBoxLayout *includeLayout = new QVBoxLayout(includeGroup);
    
    // 包含路径列表
    includePathList = new QListWidget(this);
    includePathList->setMinimumHeight(48);  // 最小高度为2行 (24px * 2)
    includePathList->setMaximumHeight(96);  // 最大高度为4行 (24px * 4)

    includeLayout->addWidget(includePathList);
    
    // 添加/删除包含路径按钮
    QHBoxLayout *includeButtonLayout = new QHBoxLayout();  // 创建水平布局
    addIncludeButton = new QPushButton(QIcon(), tr("Add Path"), this);
    removeIncludeButton = new QPushButton(QIcon(), tr("Remove Path"), this);
    includeButtonLayout->addWidget(addIncludeButton);
    includeButtonLayout->addWidget(removeIncludeButton);
    includeLayout->addLayout(includeButtonLayout);  // 将按钮布局添加到主布局
    
    // 添加宏定义设置
    QGroupBox *macroGroup = new QGroupBox(tr("Macro Definitions"), this);
    QVBoxLayout *macroLayout = new QVBoxLayout(macroGroup);
    
    // 宏定义列表
    macroList = new QListWidget(this);
    macroList->setMinimumHeight(48);  // 最小高度为2行
    macroList->setMaximumHeight(96);  // 最大高度为4行 (24px * 4)

    macroLayout->addWidget(macroList);
    
    // 添加/删除宏按钮
    QHBoxLayout *macroButtonLayout = new QHBoxLayout();  // 创建水平布局
    addMacroButton = new QPushButton(QIcon(), tr("Add Macro"), this);
    removeMacroButton = new QPushButton(QIcon(), tr("Remove Macro"), this);
    macroButtonLayout->addWidget(addMacroButton);
    macroButtonLayout->addWidget(removeMacroButton);
    macroLayout->addLayout(macroButtonLayout);  // 将按钮布局添加到主布局
    
    leftLayout->addWidget(inputGroup, 1);
    leftLayout->addWidget(includeGroup);
    leftLayout->addWidget(macroGroup);
    
    // Right panel
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(12);
    
    // 使用新的编译器设置UI
    compilerSettingUI = new CompilerSettingUI(this);
    rightLayout->addWidget(compilerSettingUI);

    // Output area
    QGroupBox *outputGroup = new QGroupBox(tr("Output"), this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    
    // 编译输出
    outputEdit = new QTextEdit(this);
    outputEdit->setReadOnly(true);
    outputLayout->addWidget(outputEdit);
    
    // 日志面板
    QHBoxLayout *logPanelLayout = new QHBoxLayout();
    logEdit = new QTextEdit(this);
    logEdit->setReadOnly(true);
    logEdit->setMaximumHeight(100);
    logEdit->setStyleSheet("QTextEdit { font-family: 'Consolas', monospace; }");
    logPanelLayout->addWidget(logEdit);
    outputLayout->addLayout(logPanelLayout);
    
    rightLayout->addWidget(outputGroup);
    
    // 添加面板到主布局
    contentLayout->addWidget(leftPanel);
    contentLayout->addWidget(rightPanel);
    
    // 设置中心部件
    setCentralWidget(centralWidget);

    inputEdit->setShaderLanguage("HLSL");
}

void DocumentWindow::setupConnections()
{
    // 包含路径按钮
    connect(addIncludeButton, &QPushButton::clicked, this, &DocumentWindow::addIncludePath);
    connect(removeIncludeButton, &QPushButton::clicked, this, &DocumentWindow::removeIncludePath);

    // 宏定义按钮
    connect(addMacroButton, &QPushButton::clicked, this, &DocumentWindow::addMacro);
    connect(removeMacroButton, &QPushButton::clicked, this, &DocumentWindow::removeMacro);
    

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

    // 监听列表内容变化，动态调整高度
    connect(includePathList, &QListWidget::itemSelectionChanged, this, [this]() {
        removeIncludeButton->setEnabled(!includePathList->selectedItems().isEmpty());
    });

    connect(includePathList->model(), &QAbstractItemModel::rowsInserted, this, [this]() {
        updateIncludeListHeight();
    });
    connect(includePathList->model(), &QAbstractItemModel::rowsRemoved, this, [this]() {
        updateIncludeListHeight();
    });

    // 监听列表内容变化，动态调整高度
    connect(macroList, &QListWidget::itemSelectionChanged, this, [this]() {
        removeMacroButton->setEnabled(!macroList->selectedItems().isEmpty());
    });

    connect(macroList->model(), &QAbstractItemModel::rowsInserted, this, [this]() {
        updateMacroListHeight();
    });
    connect(macroList->model(), &QAbstractItemModel::rowsRemoved, this, [this]() {
        updateMacroListHeight();
    });

    // 连接语言切换信号
    connect(languageCombo, &QComboBox::currentTextChanged, this, [this](const QString &language) {
        // 更新编译器设置
        compilerSettingUI->onLanguageChanged(language);

        // 保存当前编译器选择
        QString currentCompiler = compilerSettingUI->getCurrentCompiler();
        if (language == "HLSL") {
            lastGLSLCompiler = currentCompiler;
        } else {
            lastHLSLCompiler = currentCompiler;
        }
    });

    // 连接编译按钮信号
    connect(compilerSettingUI, &CompilerSettingUI::buildClicked, this, &DocumentWindow::compile);
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

    outputEdit->clear();
    logEdit->clear();

    // 根据选择的编译器创建相应的实例
    if (compiler == "FXC") {
        fxcCompiler *fxcCompilerInstance = new fxcCompiler(this);
        
        connect(fxcCompilerInstance, &fxcCompiler::compilationFinished, this, [this](const QString &output) {
            outputEdit->setTextColor(Qt::green);
            outputEdit->append(output);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::green);
            logEdit->append(currentTime + ": Compilation succeeded");
        });

        connect(fxcCompilerInstance, &fxcCompiler::compilationError, this, [this](const QString &error) {
            outputEdit->setTextColor(Qt::red);
            outputEdit->append(tr("Compilation error:\n") + error);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::red);
            logEdit->append(currentTime + ": Compilation failed");
        });

        connect(fxcCompilerInstance, &fxcCompiler::compilationWarning, this, [this](const QString &warning) {
            outputEdit->setTextColor(Qt::yellow);
            outputEdit->append(tr("Compilation warning:\n") + warning);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::yellow);
            logEdit->append(currentTime + ": Compilation warning");
        });

        fxcCompilerInstance->compile(inputEdit->toPlainText(), shaderModel, entryPoint, shaderType, includePaths, macros);
    } else if (compiler == "DXC") {
        dxcCompiler *dxcCompilerInstance = new dxcCompiler(this);
        
        connect(dxcCompilerInstance, &dxcCompiler::compilationFinished, this, [this](const QString &output) {
            outputEdit->setTextColor(Qt::green);
            outputEdit->append(output);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::green);
            logEdit->append(currentTime + ": Compilation succeeded");
        });

        connect(dxcCompilerInstance, &dxcCompiler::compilationError, this, [this](const QString &error) {
            outputEdit->setTextColor(Qt::red);
            outputEdit->append(tr("Compilation error:\n") + error);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::red);
            logEdit->append(currentTime + ": Compilation failed");
        });

        connect(dxcCompilerInstance, &dxcCompiler::compilationWarning, this, [this](const QString &warning) {
            outputEdit->setTextColor(Qt::yellow);
            outputEdit->append(tr("Compilation warning:\n") + warning);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::yellow);
            logEdit->append(currentTime + ": Compilation warning");
        });

        dxcCompilerInstance->compile(inputEdit->toPlainText(), shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    } else if (compiler == "GLSLANG") {
        glslangCompiler *glslangCompilerInstance = new glslangCompiler(this);
        
        connect(glslangCompilerInstance, &glslangCompiler::compilationFinished, this, [this](const QString &output) {
            outputEdit->setTextColor(Qt::green);
            outputEdit->append(output);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::green);
            logEdit->append(currentTime + ": Compilation succeeded");
        });

        connect(glslangCompilerInstance, &glslangCompiler::compilationError, this, [this](const QString &error) {
            outputEdit->setTextColor(Qt::red);
            outputEdit->append(tr("Compilation error:\n") + error);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::red);
            logEdit->append(currentTime + ": Compilation failed");
        });

        connect(glslangCompilerInstance, &glslangCompiler::compilationWarning, this, [this](const QString &warning) {
            outputEdit->setTextColor(Qt::yellow);
            outputEdit->append(tr("Compilation warning:\n") + warning);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::yellow);
            logEdit->append(currentTime + ": Compilation warning");
        });

        glslangCompilerInstance->compile(inputEdit->toPlainText(), languageCombo->currentText(), shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    } else if (compiler == "GLSLANGKGVER") {
        glslangkgverCompiler *glslangkgverCompilerInstance = new glslangkgverCompiler(this);
        
        connect(glslangkgverCompilerInstance, &glslangkgverCompiler::compilationFinished, this, [this](const QString &output) {
            outputEdit->setTextColor(Qt::green);
            outputEdit->append(output);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::green);
            logEdit->append(currentTime + ": Compilation succeeded");
        });

        connect(glslangkgverCompilerInstance, &glslangkgverCompiler::compilationError, this, [this](const QString &error) {
            outputEdit->setTextColor(Qt::red);
            outputEdit->append(tr("Compilation error:\n") + error);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::red);
            logEdit->append(currentTime + ": Compilation failed");
        });

        connect(glslangkgverCompilerInstance, &glslangkgverCompiler::compilationWarning, this, [this](const QString &warning) {
            outputEdit->setTextColor(Qt::yellow);
            outputEdit->append(tr("Compilation warning:\n") + warning);
            QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd-HH-mm-ss");
            logEdit->setTextColor(Qt::yellow);
            logEdit->append(currentTime + ": Compilation warning");
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
    return configPath + "/temp_docs/" + documentWindowTitle + ".ini";  // 返回完整的配置文件路径
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

void DocumentWindow::undo()
{
    if (inputEdit) {
        inputEdit->undo();
    }
}

void DocumentWindow::redo()
{
    if (inputEdit) {
        inputEdit->redo();
    }
}

