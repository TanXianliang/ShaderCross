#include "mainwindow.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtWidgets/QListWidget>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isDarkTheme(true)
    , lastHLSLCompiler("DXC")      // 设置默认值
    , lastGLSLCompiler("GLSLANG")  // 设置默认值
    , lastOpenDir(QDir::currentPath())  // 初始化为当前目录
{
    setupUI();
    createMenus();
    setupShortcuts();
    applyTheme(isDarkTheme);
    loadSettings();  // 加载上次的设置
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // Left panel
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    // Input settings area
    QGroupBox *inputGroup = new QGroupBox(tr("Shader Input"), this);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    
    // Language selection
    QHBoxLayout *langLayout = new QHBoxLayout();
    languageCombo = new QComboBox(this);
    languageCombo->addItems(QStringList() << "HLSL" << "GLSL");
    langLayout->addWidget(new QLabel(tr("Shader Language:")));
    langLayout->addWidget(languageCombo);
    inputLayout->addLayout(langLayout);
    
    // 连接着色器语言选择的信号
    connect(languageCombo, &QComboBox::currentTextChanged, this, [this](const QString &language) {
        // 保存当前编译器选择
        if (language == "HLSL") {
            lastGLSLCompiler = compilerCombo->currentText();
        } else {
            lastHLSLCompiler = compilerCombo->currentText();
        }

        // 更新编译器列表
        compilerCombo->clear();
        if (language == "HLSL") {
            compilerCombo->addItems(QStringList() << "DXC" << "FXC" << "GLSLANG");
            // 恢复上次的HLSL编译器选择
            int index = compilerCombo->findText(lastHLSLCompiler);
            if (index >= 0) {
                compilerCombo->setCurrentIndex(index);
            }
        } else if (language == "GLSL") {
            compilerCombo->addItems(QStringList() << "GLSLANG");
            // 恢复上次的GLSL编译器选择
            int index = compilerCombo->findText(lastGLSLCompiler);
            if (index >= 0) {
                compilerCombo->setCurrentIndex(index);
            }
        }
    });
    
    // File selection
    QHBoxLayout *fileLayout = new QHBoxLayout();
    filePathEdit = new QLineEdit(this);
    browseButton = new QPushButton(tr("Browse"), this);
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

    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseFile);
    connect(encodingCombo, &QComboBox::currentTextChanged, [this]() {
        if (!filePathEdit->text().isEmpty()) {
            loadFileContent(filePathEdit->text());
        }
    });

    // 添加文件内容显示区域
    inputEdit = new QTextEdit(this);
    inputEdit->setReadOnly(false);  // 改为可编辑
    inputLayout->addWidget(inputEdit);
    
    // 添加包含路径设置
    QGroupBox *includeGroup = new QGroupBox(tr("Include Paths"), this);
    QVBoxLayout *includeLayout = new QVBoxLayout(includeGroup);
    
    // 包含路径列表
    includePathList = new QListWidget(this);
    includeLayout->addWidget(includePathList);
    
    // 添加/删除按钮
    QHBoxLayout *includeButtonLayout = new QHBoxLayout();
    addIncludeButton = new QPushButton(tr("Add Path"), this);
    removeIncludeButton = new QPushButton(tr("Remove Path"), this);
    includeButtonLayout->addWidget(addIncludeButton);
    includeButtonLayout->addWidget(removeIncludeButton);
    includeLayout->addLayout(includeButtonLayout);
    
    // 连接信号
    connect(addIncludeButton, &QPushButton::clicked, this, &MainWindow::onAddIncludePath);
    connect(removeIncludeButton, &QPushButton::clicked, this, &MainWindow::onRemoveIncludePath);
    
    // 添加宏定义设置
    QGroupBox *macroGroup = new QGroupBox(tr("Macro Definitions"), this);
    QVBoxLayout *macroLayout = new QVBoxLayout(macroGroup);
    
    // 宏定义列表
    macroList = new QListWidget(this);
    macroLayout->addWidget(macroList);
    
    // 添加/删除按钮
    QHBoxLayout *macroButtonLayout = new QHBoxLayout();
    addMacroButton = new QPushButton(tr("Add Macro"), this);
    removeMacroButton = new QPushButton(tr("Remove Macro"), this);
    macroButtonLayout->addWidget(addMacroButton);
    macroButtonLayout->addWidget(removeMacroButton);
    macroLayout->addLayout(macroButtonLayout);
    
    // 连接信号
    connect(addMacroButton, &QPushButton::clicked, this, &MainWindow::onAddMacro);
    connect(removeMacroButton, &QPushButton::clicked, this, &MainWindow::onRemoveMacro);
    
    leftLayout->addWidget(inputGroup);
    leftLayout->addWidget(includeGroup);
    leftLayout->addWidget(macroGroup);
    leftLayout->addStretch();
    
    // Right panel
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    // 编译器选项
    QGroupBox *compilerGroup = new QGroupBox(tr("Compiler Options"), this);
    QVBoxLayout *compilerLayout = new QVBoxLayout(compilerGroup);
    
    // 编译工具选择
    QHBoxLayout *compilerToolLayout = new QHBoxLayout();
    compilerToolLayout->addWidget(new QLabel(tr("Compiler:")));
    compilerCombo = new QComboBox(this);
    // 初始化编译器列表（根据默认的着色器语言）
    if (languageCombo->currentText() == "HLSL") {
        compilerCombo->addItems(QStringList() << "DXC" << "FXC" << "GLSLANG");
    } else {
        compilerCombo->addItems(QStringList() << "GLSLANG");
    }
    compilerToolLayout->addWidget(compilerCombo);
    compilerLayout->addLayout(compilerToolLayout);
    
    // Shader类型选择
    QHBoxLayout *shaderTypeLayout = new QHBoxLayout();
    shaderTypeLayout->addWidget(new QLabel(tr("Shader Type:")));
    shaderTypeCombo = new QComboBox(this);
    shaderTypeCombo->addItems(QStringList() 
        << "Vertex" 
        << "Pixel" 
        << "Geometry" 
        << "Compute" 
        << "Hull" 
        << "Domain"
        << "RayGeneration"      // 光线生成着色器
        << "Intersection"       // 相交着色器
        << "AnyHit"            // 任意命中着色器
        << "ClosestHit"        // 最近命中着色器
        << "Miss"              // 未命中着色器
        << "Callable"          // 可调用着色器
        << "Mesh"              // 网格着色器
    );
    shaderTypeLayout->addWidget(shaderTypeCombo);
    compilerLayout->addLayout(shaderTypeLayout);
    
    // 入口点设置
    QHBoxLayout *entryPointLayout = new QHBoxLayout();
    entryPointLayout->addWidget(new QLabel(tr("Entry Point:")));
    entryPointEdit = new QLineEdit(this);
    entryPointEdit->setPlaceholderText(tr("main"));
    entryPointEdit->setText("main");
    entryPointLayout->addWidget(entryPointEdit);
    compilerLayout->addLayout(entryPointLayout);
    
    // Shader Model选择
    QHBoxLayout *shaderModelLayout = new QHBoxLayout();
    shaderModelLayout->addWidget(new QLabel(tr("Shader Model:")));
    shaderModelCombo = new QComboBox(this);
    shaderModelCombo->addItems(QStringList() 
        << "5.0"
        << "5.1"
        << "6.0"
        << "6.4"
    );
    shaderModelLayout->addWidget(shaderModelCombo);
    compilerLayout->addLayout(shaderModelLayout);
    
    // 根据编译器选择更新界面
    connect(compilerCombo, &QComboBox::currentTextChanged, this, [this](const QString &compiler) {
        // TODO: 根据不同编译器更新相关选项
        if (compiler == "DXC") {
            // DXC特有选项
        } else if (compiler == "FXC") {
            // FXC特有选项
        } else if (compiler == "GLSLANG") {
            // GLSLANG特有选项
        }
    });
    
    rightLayout->addWidget(compilerGroup);
    
    // Output area
    QGroupBox *outputGroup = new QGroupBox(tr("Output"), this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    outputEdit = new QTextEdit(this);
    outputLayout->addWidget(outputEdit);
    rightLayout->addWidget(outputGroup);
    
    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 1);
    
    setCentralWidget(centralWidget);
    resize(1200, 800);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("Open"), this, &MainWindow::onBrowseFile, QKeySequence::Open);
    fileMenu->addAction(tr("Save"), this, &MainWindow::onSaveResult, QKeySequence::Save);
    
    QMenu *editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(tr("Copy"), this, [this](){ outputEdit->copy(); }, QKeySequence::Copy);
    editMenu->addAction(tr("Paste"), this, [this](){ outputEdit->paste(); }, QKeySequence::Paste);
    
    QMenu *buildMenu = menuBar()->addMenu(tr("Build"));
    buildMenu->addAction(tr("Compile"), this, &MainWindow::onCompile, Qt::Key_F5);
    buildMenu->addAction(tr("Show Disassembly"), this, &MainWindow::onShowDisassembly, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));

    QMenu *uiMenu = menuBar()->addMenu(tr("UI"));
    uiMenu->addAction(tr("Reset Layout"), this, &MainWindow::onResetLayout);
    uiMenu->addAction(tr("Toggle Output Panel"), this, &MainWindow::onToggleOutput, Qt::Key_F12);
    uiMenu->addSeparator();
    uiMenu->addAction(tr("Toggle Theme"), this, &MainWindow::onToggleTheme, QKeySequence(Qt::CTRL | Qt::Key_T));
}

void MainWindow::setupShortcuts()
{
    // 快捷键已在createMenus()中设置
}

void MainWindow::loadFileContent(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setCodec(encodingCombo->currentText().toLocal8Bit());
        inputEdit->setText(in.readAll());
        file.close();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open file"));
    }
}

void MainWindow::onBrowseFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        tr("Select Shader File"), 
        lastOpenDir,  // 使用上次的目录
        tr("All Files (*.*);;Shader Files (*.hlsl *.glsl)")
    );
    if (!fileName.isEmpty()) {
        filePathEdit->setText(fileName);
        lastOpenDir = QFileInfo(fileName).absolutePath();  // 保存新的目录
        loadFileContent(fileName);
    }
}

void MainWindow::onCompile()
{
    QMessageBox::information(this, tr("Notice"), tr("Compilation not implemented yet"));
}

void MainWindow::onSaveResult()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Compilation Result"), QString(), tr("All Files (*.*)"));
    if (!fileName.isEmpty()) {
        // TODO: Implement save logic
    }
}

void MainWindow::onShowDisassembly()
{
    QMessageBox::information(this, tr("Notice"), tr("Disassembly not implemented yet"));
}

void MainWindow::onResetLayout()
{
    resize(1200, 800);
    // TODO: 重置其他布局设置
}

void MainWindow::onToggleOutput()
{
    if (outputEdit->parentWidget()->parentWidget()->isVisible()) {
        outputEdit->parentWidget()->parentWidget()->hide();
    } else {
        outputEdit->parentWidget()->parentWidget()->show();
    }
}

void MainWindow::onToggleTheme()
{
    isDarkTheme = !isDarkTheme;
    applyTheme(isDarkTheme);
}

void MainWindow::onAddIncludePath()
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

void MainWindow::onRemoveIncludePath()
{
    QListWidgetItem *currentItem = includePathList->currentItem();
    if (currentItem) {
        delete includePathList->takeItem(includePathList->row(currentItem));
    }
}

void MainWindow::showMacroDialog()
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
            } else {
                QMessageBox::warning(this, tr("Warning"), tr("A macro with this name already exists."));
            }
        }
    }
}

void MainWindow::onAddMacro()
{
    showMacroDialog();
}

void MainWindow::onRemoveMacro()
{
    QListWidgetItem *currentItem = macroList->currentItem();
    if (currentItem) {
        delete macroList->takeItem(macroList->row(currentItem));
    }
}

void MainWindow::applyTheme(bool dark)
{
    if (dark) {
        // 暗色主题
        QString darkStyle = R"(
            QMainWindow, QWidget { background-color: #2b2b2b; color: #d4d4d4; }
            QGroupBox { 
                background-color: #3c3f41; 
                border: 1px solid #555555;
                border-radius: 3px;
                margin-top: 0.5em;
                padding-top: 0.5em;
            }
            QGroupBox::title {
                color: #d4d4d4;
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 3px 0 3px;
            }
            QTextEdit, QLineEdit, QComboBox {
                background-color: #3c3f41;
                border: 1px solid #555555;
                color: #d4d4d4;
                selection-background-color: #214283;  /* 添加选中文本的背景色 */
                selection-color: #ffffff;            /* 添加选中文本的前景色 */
            }
            QPushButton {
                background-color: #4c5052;
                border: 1px solid #555555;
                color: #d4d4d4;
                padding: 4px 8px;
            }
            QPushButton:hover {
                background-color: #565656;
            }
            QMenuBar {
                background-color: #3c3f41;
                color: #d4d4d4;
            }
            QMenuBar::item:selected {
                background-color: #4b6eaf;
            }
            QMenu {
                background-color: #3c3f41;
                color: #d4d4d4;
                border: 1px solid #555555;
            }
            QMenu::item:selected {
                background-color: #4b6eaf;
            }
        )";
        setStyleSheet(darkStyle);
    } else {
        // 恢复默认主题
        setStyleSheet("");
    }
}

MainWindow::~MainWindow()
{
    saveSettings();  // 保存当前设置
}

QString MainWindow::settingsFilePath() const
{
    // 使用程序所在目录下的config目录
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + "/config";
    QDir().mkpath(configPath);  // 确保config目录存在
    return configPath + "/settings.ini";
}

void MainWindow::saveSettings()
{
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    
    // 保存基本设置
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("isDarkTheme", isDarkTheme);
    settings.setValue("shaderLanguage", languageCombo->currentText());
    settings.setValue("filePath", filePathEdit->text());
    settings.setValue("encoding", encodingCombo->currentText());
    settings.setValue("entryPoint", entryPointEdit->text());
    settings.setValue("shaderType", shaderTypeCombo->currentText());
    settings.setValue("shaderModel", shaderModelCombo->currentText());
    
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
    
    // 保存文件内容
    settings.setValue("inputContent", inputEdit->toPlainText());
    settings.setValue("outputContent", outputEdit->toPlainText());
    
    // 保存编译器选择
    settings.setValue("lastHLSLCompiler", lastHLSLCompiler);
    settings.setValue("lastGLSLCompiler", lastGLSLCompiler);
    settings.setValue("lastOpenDir", lastOpenDir);  // 保存目录
}

void MainWindow::loadSettings()
{
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    
    // 恢复基本设置
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    bool darkTheme = settings.value("isDarkTheme", true).toBool();
    if (darkTheme != isDarkTheme) {
        isDarkTheme = darkTheme;
        applyTheme(isDarkTheme);
    }
    
    QString lang = settings.value("shaderLanguage").toString();
    if (!lang.isEmpty()) {
        languageCombo->setCurrentText(lang);
    }
    
    QString encoding = settings.value("encoding").toString();
    if (!encoding.isEmpty()) {
        encodingCombo->setCurrentText(encoding);
    }
    
    // 恢复包含路径
    QStringList includePaths = settings.value("includePaths").toStringList();
    includePathList->clear();
    includePathList->addItems(includePaths);
    
    // 恢复宏定义
    QStringList macros = settings.value("macros").toStringList();
    macroList->clear();
    macroList->addItems(macros);
    
    // 恢复文件内容
    QString filePath = settings.value("filePath").toString();
    if (!filePath.isEmpty()) {
        filePathEdit->setText(filePath);
        if (QFile::exists(filePath)) {
            loadFileContent(filePath);
        } else {
            // 如果文件不存在，使用保存的内容
            inputEdit->setText(settings.value("inputContent").toString());
        }
    }
    
    outputEdit->setText(settings.value("outputContent").toString());
    
    // 加载编译器选择
    lastHLSLCompiler = settings.value("lastHLSLCompiler", "DXC").toString();
    lastGLSLCompiler = settings.value("lastGLSLCompiler", "GLSLANG").toString();
    
    // 根据当前语言设置对应的编译器
    if (languageCombo->currentText() == "HLSL") {
        int index = compilerCombo->findText(lastHLSLCompiler);
        if (index >= 0) {
            compilerCombo->setCurrentIndex(index);
        }
    } else {
        int index = compilerCombo->findText(lastGLSLCompiler);
        if (index >= 0) {
            compilerCombo->setCurrentIndex(index);
        }
    }
    
    // 加载入口点
    QString entryPoint = settings.value("entryPoint", "main").toString();
    entryPointEdit->setText(entryPoint);
    
    // 加载shader类型
    QString shaderType = settings.value("shaderType", "Vertex").toString();
    shaderTypeCombo->setCurrentText(shaderType);
    
    // 加载shader模型
    QString shaderModel = settings.value("shaderModel", "5.0").toString();
    shaderModelCombo->setCurrentText(shaderModel);
    
    lastOpenDir = settings.value("lastOpenDir", QDir::currentPath()).toString();  // 加载目录
} 