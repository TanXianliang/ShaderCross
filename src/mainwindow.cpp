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
#include <QtWidgets/QPushButton>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStatusBar>
#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include "fxcCompiler.h"
#include "compilerConfig.h"
#include <QDebug>
#include "languageConfig.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isDarkTheme(true)
    , lastHLSLCompiler("DXC")      // 设置默认值
    , lastGLSLCompiler("GLSLANG")  // 设置默认值
    , lastOpenDir(QDir::currentPath())  // 初始化为当前目录
    , compiler(new fxcCompiler(this))
{
    // 设置程序图标
    setWindowIcon(QIcon(":/resources/icons.jpg"));
    
    // 设置窗口标题
    setWindowTitle("ShaderCross");

    setupUI();
    createMenus();
    setupShortcuts();
    applyTheme(isDarkTheme);
    loadSettings();  // 加载上次的设置

    connect(compiler, &fxcCompiler::compilationFinished, this, [this](const QString &output) {
        logEdit->setTextColor(Qt::green);
        logEdit->append(tr("Compilation succeeded:\n") + output);
    });

    connect(compiler, &fxcCompiler::compilationError, this, [this](const QString &error) {
        logEdit->setTextColor(Qt::red);
        logEdit->append(tr("Compilation error:\n") + error);
    });
}

void MainWindow::setupUI()
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
    languageCombo->addItems(QStringList() << "HLSL" << "GLSL");
    langLayout->addWidget(new QLabel(tr("Shader Language:")));
    langLayout->addWidget(languageCombo);
    inputLayout->addLayout(langLayout);
    
    // File selection
    QHBoxLayout *fileLayout = new QHBoxLayout();
    filePathEdit = new QLineEdit(this);
    browseButton = new QPushButton(QIcon(":/resources/icons/browse.svg"), tr("Browse"), this);
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

    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseFile);
    connect(encodingCombo, &QComboBox::currentTextChanged, [this]() {
        if (!filePathEdit->text().isEmpty()) {
            loadFileContent(filePathEdit->text());
        }
    });

    // 添加文件内容显示区域
    inputEdit = new QTextEdit(this);
    inputEdit->setReadOnly(false);
    inputLayout->addWidget(inputEdit, 1);  // 添加拉伸因子1
    
    // 添加包含路径设置
    QGroupBox *includeGroup = new QGroupBox(tr("Include Paths"), this);
    QVBoxLayout *includeLayout = new QVBoxLayout(includeGroup);
    
    // 包含路径列表
    includePathList = new QListWidget(this);
    includePathList->setMinimumHeight(48);  // 最小高度为2行 (24px * 2)
    includePathList->setMaximumHeight(96);  // 最大高度为4行 (24px * 4)
    
    // 监听列表内容变化，动态调整高度
    connect(includePathList->model(), &QAbstractItemModel::rowsInserted, this, [this]() {
        updateIncludeListHeight();
    });
    connect(includePathList->model(), &QAbstractItemModel::rowsRemoved, this, [this]() {
        updateIncludeListHeight();
    });
    
    includeLayout->addWidget(includePathList);
    
    // 添加/删除包含路径按钮
    QHBoxLayout *includeButtonLayout = new QHBoxLayout();  // 创建水平布局
    addIncludeButton = new QPushButton(QIcon(":/resources/icons/add.svg"), tr("Add Path"), this);
    removeIncludeButton = new QPushButton(QIcon(":/resources/icons/remove.svg"), tr("Remove Path"), this);
    includeButtonLayout->addWidget(addIncludeButton);
    includeButtonLayout->addWidget(removeIncludeButton);
    includeLayout->addLayout(includeButtonLayout);  // 将按钮布局添加到主布局
    
    // 连接包含路径按钮的信号
    connect(addIncludeButton, &QPushButton::clicked, this, &MainWindow::onAddIncludePath);
    connect(removeIncludeButton, &QPushButton::clicked, this, &MainWindow::onRemoveIncludePath);
    
    // 添加宏定义设置
    QGroupBox *macroGroup = new QGroupBox(tr("Macro Definitions"), this);
    QVBoxLayout *macroLayout = new QVBoxLayout(macroGroup);
    
    // 宏定义列表
    macroList = new QListWidget(this);
    macroList->setMinimumHeight(48);  // 最小高度为2行
    macroList->setMaximumHeight(96);  // 最大高度为4行 (24px * 4)
    
    // 监听列表内容变化，动态调整高度
    connect(macroList->model(), &QAbstractItemModel::rowsInserted, this, [this]() {
        updateMacroListHeight();
    });
    connect(macroList->model(), &QAbstractItemModel::rowsRemoved, this, [this]() {
        updateMacroListHeight();
    });
    
    macroLayout->addWidget(macroList);
    
    // 添加/删除宏按钮
    QHBoxLayout *macroButtonLayout = new QHBoxLayout();  // 创建水平布局
    addMacroButton = new QPushButton(QIcon(":/resources/icons/add.svg"), tr("Add Macro"), this);
    removeMacroButton = new QPushButton(QIcon(":/resources/icons/remove.svg"), tr("Remove Macro"), this);
    macroButtonLayout->addWidget(addMacroButton);
    macroButtonLayout->addWidget(removeMacroButton);
    macroLayout->addLayout(macroButtonLayout);  // 将按钮布局添加到主布局
    
    // 连接宏定义按钮的信号
    connect(addMacroButton, &QPushButton::clicked, this, &MainWindow::onAddMacro);
    connect(removeMacroButton, &QPushButton::clicked, this, &MainWindow::onRemoveMacro);
    
    leftLayout->addWidget(inputGroup, 1);
    leftLayout->addWidget(includeGroup);
    leftLayout->addWidget(macroGroup);
    
    // Right panel
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(12);
    
    // 修改编译器组的布局
    QGroupBox *compilerGroup = new QGroupBox(tr("Compiler Settings"), this);
    QVBoxLayout *compilerLayout = new QVBoxLayout(compilerGroup);
    
    // 编译器选择
    QHBoxLayout *compilerSelectLayout = new QHBoxLayout();
    compilerSelectLayout->addWidget(new QLabel(tr("Compiler:")));
    compilerCombo = new QComboBox(this);
    compilerCombo->addItems(QStringList() << "FXC" << "DXC" << "GLSLANG" << "SPIRV-CROSS");
    compilerSelectLayout->addWidget(compilerCombo);
    compilerLayout->addLayout(compilerSelectLayout);

    // 更新着色器类型选项
    connect(compilerCombo, &QComboBox::currentTextChanged, this, [this](const QString &compiler) {
        shaderTypeCombo->clear();
        
        if (CompilerConfig::instance().hasCompiler(compiler)) {
            const auto& capability = CompilerConfig::instance().getCapability(compiler);
            shaderTypeCombo->addItems(capability.supportedShaderTypes);
            
            // 同时更新输出类型选项
            outputTypeCombo->clear();
            outputTypeCombo->addItems(capability.supportedOutputTypes);
            
            // 更新 Shader Model 选项
            shaderModelCombo->clear();
            shaderModelCombo->addItems(capability.supportedShaderModels);
        }
    });
    
    // Shader类型选择
    QHBoxLayout *shaderTypeLayout = new QHBoxLayout();
    shaderTypeLayout->addWidget(new QLabel(tr("Shader Type:")));
    shaderTypeCombo = new QComboBox(this);
    
    // 根据当前编译器设置着色器类型选项
    QString currentCompiler = compilerCombo->currentText();
    if (CompilerConfig::instance().hasCompiler(currentCompiler)) {
        const auto& capability = CompilerConfig::instance().getCapability(currentCompiler);
        shaderTypeCombo->addItems(capability.supportedShaderTypes);
    }
    
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
    
    // 根据当前编译器设置 Shader Model 选项
    if (CompilerConfig::instance().hasCompiler(currentCompiler)) {
        const auto& capability = CompilerConfig::instance().getCapability(currentCompiler);
        shaderModelCombo->addItems(capability.supportedShaderModels);
    }
    
    shaderModelLayout->addWidget(shaderModelCombo);
    compilerLayout->addLayout(shaderModelLayout);
    
    // 输出类型选择
    QHBoxLayout *outputTypeLayout = new QHBoxLayout();
    outputTypeLayout->addWidget(new QLabel(tr("Output Type:")));
    outputTypeCombo = new QComboBox(this);
    
    // 根据当前编译器设置输出类型选项
    if (CompilerConfig::instance().hasCompiler(currentCompiler)) {
        const auto& capability = CompilerConfig::instance().getCapability(currentCompiler);
        // 添加安全检查
        if (!capability.supportedOutputTypes.isEmpty()) {
            qDebug() << "Adding output types for" << currentCompiler << ":" << capability.supportedOutputTypes;
            auto supportedOutputTypes = capability.supportedOutputTypes;
            outputTypeCombo->addItems(supportedOutputTypes);
        } else {
            qDebug() << "Warning: No output types available for" << currentCompiler;
            outputTypeCombo->addItem("Default");  // 添加一个默认选项
        }
    } else {
        qDebug() << "Warning: Compiler not found:" << currentCompiler;
        outputTypeCombo->addItem("Default");  // 添加一个默认选项
    }
    
    outputTypeLayout->addWidget(outputTypeCombo);
    compilerLayout->addLayout(outputTypeLayout);

    // 添加构建按钮
    QHBoxLayout *buildLayout = new QHBoxLayout();
    buildLayout->addStretch();  // 添加弹性空间，使按钮靠右对齐
    buildButton = new QPushButton(tr("Build"), this);
    buildButton->setObjectName("buildButton");  // 设置对象名，用于样式表定位
    buildButton->setFixedWidth(100);
    buildLayout->addWidget(buildButton);
    compilerLayout->addLayout(buildLayout);

    // 连接构建按钮的点击信号
    connect(buildButton, &QPushButton::clicked, this, [this]() {
        QString inputFile = filePathEdit->text();
        QString shaderModel = shaderModelCombo->currentText();
        QString entryPoint = entryPointEdit->text();
        QString shaderType = shaderTypeCombo->currentText();
        
        // 获取包含路径列表
        QStringList includePaths;
        for (int i = 0; i < includePathList->count(); ++i) {
            includePaths << includePathList->item(i)->text();
        }
        
        // 获取宏定义列表
        QStringList macros;
        for (int i = 0; i < macroList->count(); ++i) {
            macros << macroList->item(i)->text();
        }

        // 调用更新后的compile方法
        compiler->compile(inputFile, shaderModel, entryPoint, shaderType, includePaths, macros);
    });
    
    rightLayout->addWidget(compilerGroup);
    
    // Output area
    QGroupBox *outputGroup = new QGroupBox(tr("Output"), this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    
    // 编译输出
    outputEdit = new QTextEdit(this);
    outputEdit->setReadOnly(true);  // 设置为只读
    outputLayout->addWidget(outputEdit);
    
    // 添加分隔线
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    outputLayout->addWidget(line);
    
    // 日志面板
    QHBoxLayout *logPanelLayout = new QHBoxLayout();
    logPanelLayout->addWidget(new QLabel(tr("Log:")));
    logEdit = new QTextEdit(this);
    logEdit->setReadOnly(true);  // 设置为只读
    logEdit->setMaximumHeight(100);  // 限制最大高度
    logEdit->setStyleSheet("QTextEdit { font-family: 'Consolas', monospace; }");  // 使用等宽字体
    logPanelLayout->addWidget(logEdit);
    outputLayout->addLayout(logPanelLayout);
    
    rightLayout->addWidget(outputGroup);
    
    // 添加面板到主布局
    contentLayout->addWidget(leftPanel);
    contentLayout->addWidget(rightPanel);
    
    // 设置中心部件
    setCentralWidget(centralWidget);
    
    // 语言切换时更新编译器选项
    connect(languageCombo, &QComboBox::currentTextChanged, this, [this](const QString &language) {
        // 保存当前编译器选择
        if (language == "HLSL") {
            lastGLSLCompiler = compilerCombo->currentText();
        } else {
            lastHLSLCompiler = compilerCombo->currentText();
        }

        compilerCombo->clear();
        if (LanguageConfig::instance().hasLanguage(language)) {
            QStringList compilers = LanguageConfig::instance().getSupportedCompilers(language);
            compilerCombo->addItems(compilers);
            
            // 恢复上次的编译器选择
            QString lastCompiler = (language == "HLSL") ? lastHLSLCompiler : lastGLSLCompiler;
            int index = compilerCombo->findText(lastCompiler);
            if (index >= 0) {
                compilerCombo->setCurrentIndex(index);
            }
        }
    });
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
        // 现代暗色主题
        QString darkStyle = R"(
            QMainWindow, QWidget {
                background-color: #1e1e1e;
                color: #d4d4d4;
            }
            
            QGroupBox { 
                background-color: #252526;
                border: 1px solid #2d2d2d;
                border-radius: 6px;
                margin-top: 1.5em;  /* 增加顶部边距，给标题留出空间 */
                padding: 1.2em 8px 8px 8px;  /* 增加顶部内边距 */
                font-weight: bold;
            }
            
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                top: 0.5em;  /* 调整标题位置 */
                padding: 0 5px;
                background-color: #252526;
                color: #0098ff;
            }
            
            QTextEdit {
                background-color: #1e1e1e;
                border: 1px solid #2d2d2d;
                border-radius: 4px;
                padding: 4px;
                selection-background-color: #264f78;
                selection-color: #ffffff;
                font-family: "Consolas", "Source Code Pro", monospace;
            }
            
            QLineEdit, QComboBox {
                background-color: #3c3c3c;
                border: 1px solid #2d2d2d;
                border-radius: 4px;
                padding: 4px 8px;
                min-height: 24px;
                selection-background-color: #264f78;
            }
            
            QComboBox::drop-down {
                border: none;
                width: 24px;
            }
            
            /* 所有按钮使用统一的灰蓝色样式 */
            QPushButton {
                background-color: #3c5c84 !important;
                border: none !important;
                border-radius: 4px !important;
                padding: 6px 12px !important;
                color: white !important;
                font-weight: bold !important;
                min-height: 24px !important;
            }
            
            QPushButton:hover {
                background-color: #4a6c94 !important;
            }
            
            QPushButton:pressed {
                background-color: #2e4c74 !important;
            }
            
            QPushButton:disabled {
                background-color: #2d4d74 !important;
                color: #a0a0a0 !important;
            }
            
            QPushButton:focus {
                background-color: #3c5c84 !important;
                outline: none !important;
            }

            /* 标题栏样式 */
            QMenuBar {
                background-color: #252526;
                border-bottom: 1px solid #2d2d2d;
                padding: 2px;
                min-height: 28px;
            }
            
            QMenuBar::item {
                background: transparent;
                padding: 4px 8px;
                border-radius: 4px;
                margin: 2px;
                color: #d4d4d4;
            }
            
            QMenuBar::item:selected {
                background-color: #323233;
                color: #ffffff;
            }
            
            QMenuBar::item:pressed {
                background-color: #2d2d2d;
                color: #ffffff;
            }
            
            QMenu {
                background-color: #252526;
                border: 1px solid #2d2d2d;
                border-radius: 4px;
                padding: 4px;
            }
            
            QMenu::item {
                padding: 6px 24px;
                border-radius: 2px;
                min-width: 150px;
                color: #d4d4d4;
            }
            
            QMenu::item:selected {
                background-color: #323233;
                color: #ffffff;
            }
            
            QMenu::separator {
                height: 1px;
                background-color: #2d2d2d;
                margin: 4px 0;
            }

            /* 窗口控制按钮样式 */
            QPushButton#windowButton {
                background: transparent;
                border: none;
                color: #d4d4d4;
                font-family: "Segoe UI", sans-serif;
                font-size: 14px;
            }

            QPushButton#windowButton:hover {
                background-color: #3c3c3c;
            }

            QPushButton#closeButton:hover {
                background-color: #c42b1c;
                color: white;
            }
        )";
        setStyleSheet(darkStyle);
    } else {
        // 浅色主题
        QString lightStyle = R"(
            QGroupBox { 
                margin-top: 1.5em;
                padding: 1.2em 8px 8px 8px;
            }
            
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                top: 0.5em;
                padding: 0 5px;
            }

            QComboBox::drop-down {
                border: none;
                width: 24px;
            }

            /* 所有按钮使用统一的灰蓝色样式 */
            QPushButton {
                background-color: #3c5c84 !important;
                border: none !important;
                border-radius: 4px !important;
                padding: 6px 12px !important;
                color: white !important;
                font-weight: bold !important;
                min-height: 24px !important;
            }
            
            QPushButton:hover {
                background-color: #4a6c94 !important;
            }
            
            QPushButton:pressed {
                background-color: #2e4c74 !important;
            }
            
            QPushButton:disabled {
                background-color: #2d4d74 !important;
                color: #a0a0a0 !important;
            }
            
            QPushButton:focus {
                background-color: #3c5c84 !important;
                outline: none !important;
            }

            /* 标题栏样式 */
            QMenuBar {
                background-color: #f5f5f5;
                border-bottom: 1px solid #e0e0e0;
                padding: 2px;
                min-height: 28px;
            }
            
            QMenuBar::item {
                background: transparent;
                padding: 4px 8px;
                border-radius: 4px;
                margin: 2px;
            }
            
            QMenuBar::item:selected {
                background-color: #e8e8e8;
            }
            
            QMenuBar::item:pressed {
                background-color: #d8d8d8;
            }
            
            QMenu {
                background-color: #ffffff;
                border: 1px solid #e0e0e0;
                border-radius: 4px;
                padding: 4px;
            }
            
            QMenu::item {
                padding: 6px 24px;
                border-radius: 2px;
                min-width: 150px;
            }
            
            QMenu::item:selected {
                background-color: #e8e8e8;
            }
            
            QMenu::separator {
                height: 1px;
                background-color: #e0e0e0;
                margin: 4px 0;
            }

            /* 标题栏样式 */
            QWidget#titleBar {
                background-color: #f5f5f5;
                border-bottom: 1px solid #e0e0e0;
            }

            QWidget#titleBar QLabel {
                color: #000000;
            }
        )";
        setStyleSheet(lightStyle);
    }
}

MainWindow::~MainWindow()
{
    QSettings settings("config/settings.ini", QSettings::IniFormat);
    
    // 保存编译器选择
    settings.setValue("lastHLSLCompiler", lastHLSLCompiler);
    settings.setValue("lastGLSLCompiler", lastGLSLCompiler);
    
    // 保存当前编辑器内容
    settings.setValue("inputContent", inputEdit->toPlainText());
    settings.setValue("outputContent", outputEdit->toPlainText());
    
    // 保存编译设置
    settings.setValue("entryPoint", entryPointEdit->text());
    settings.setValue("shaderType", shaderTypeCombo->currentText());
    settings.setValue("shaderModel", shaderModelCombo->currentText());
    settings.setValue("outputType", outputTypeCombo->currentText());
    
    // 保存最后打开的目录
    settings.setValue("lastOpenDir", lastOpenDir);
    
    // 保存主题设置
    settings.setValue("isDarkTheme", isDarkTheme);
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
    settings.setValue("outputType", outputTypeCombo->currentText());
    
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
    
    // 保存编译器选项
    settings.setValue("compiler", compilerCombo->currentText());
    settings.setValue("lastHLSLCompiler", lastHLSLCompiler);
    settings.setValue("lastGLSLCompiler", lastGLSLCompiler);
    
    // 保存最后打开的目录
    settings.setValue("lastOpenDir", lastOpenDir);
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
    
    // 加载编译器选项
    QString compiler = settings.value("compiler").toString();
    if (!compiler.isEmpty()) {
        compilerCombo->setCurrentText(compiler);
    }
    
    // 加载编译器选择
    lastHLSLCompiler = settings.value("lastHLSLCompiler", "DXC").toString();
    lastGLSLCompiler = settings.value("lastGLSLCompiler", "GLSLANG").toString();
    
    // 根据当前语言设置对应的编译器
    if (languageCombo->currentText() == "HLSL") {
        compilerCombo->setCurrentText(lastHLSLCompiler);
    } else {
        compilerCombo->setCurrentText(lastGLSLCompiler);
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
    
    // 加载输出类型
    QString outputType = settings.value("outputType", "DXIL").toString();
    outputTypeCombo->setCurrentText(outputType);
    
    lastOpenDir = settings.value("lastOpenDir", QDir::currentPath()).toString();  // 加载目录
}

void MainWindow::updateIncludeListHeight()
{
    const int rowHeight = 24;  // 每行的高度
    const int minRows = 2;     // 最少显示行数
    const int maxRows = 4;     // 最多显示行数（改为4）
    
    // 计算当前内容需要的行数
    int itemCount = includePathList->count();
    int neededRows = qBound(minRows, itemCount, maxRows);
    
    // 设置新的高度（加上一些边距）
    int newHeight = neededRows * rowHeight + 8;  // 8px for padding
    includePathList->setFixedHeight(newHeight);
}

void MainWindow::updateMacroListHeight()
{
    const int rowHeight = 24;  // 每行的高度
    const int minRows = 2;     // 最少显示行数
    const int maxRows = 4;     // 最多显示行数（改为4）
    
    // 计算当前内容需要的行数
    int itemCount = macroList->count();
    int neededRows = qBound(minRows, itemCount, maxRows);
    
    // 设置新的高度（加上一些边距）
    int newHeight = neededRows * rowHeight + 8;  // 8px for padding
    macroList->setFixedHeight(newHeight);
} 