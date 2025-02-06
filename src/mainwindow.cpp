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
#include "compilerSettingUI.h"
#include "dxcCompiler.h"
#include "glslangCompiler.h"
#include "shaderCodeTextEdit.h"
#include "glslangkgverCompiler.h"

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

    // 连接编译器信号
    connect(compiler, &fxcCompiler::compilationFinished, this, [this](const QString &output) {
        outputEdit->setTextColor(Qt::green);
        outputEdit->append(tr("Compilation succeeded:\n") + output);
        logEdit->setTextColor(Qt::green);
        logEdit->append(tr("Compilation succeeded"));
    });

    connect(compiler, &fxcCompiler::compilationError, this, [this](const QString &error) {
        outputEdit->setTextColor(Qt::red);
        outputEdit->append(tr("Compilation error:\n") + error);
        logEdit->setTextColor(Qt::red);
        logEdit->append(tr("Compilation failed"));
    });

    // 连接编译器设置变更信号
    connect(compilerSettingUI, &CompilerSettingUI::compilerChanged, this, [this](const QString &compiler) {
        if (languageCombo->currentText() == "HLSL") {
            lastHLSLCompiler = compiler;
        } else {
            lastGLSLCompiler = compiler;
        }
        // 更新当前编译器设置
        updateCurrentCompilerSettings(compiler);
        inputEdit->setShaderLanguage(languageCombo->currentText()); // 更新输入编辑器语言
    });

    // 在 MainWindow 类中添加 glslangCompiler 的实例
    glslangCompiler *glslangCompilerInstance = new glslangCompiler(this);

    // 连接信号
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

    // 连接 glslangkgverCompiler 的信号
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

    QStringList languages = LanguageConfig::instance().getSupportedLanguages();
    languageCombo->addItems(languages);
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
    
    // 使用新的编译器设置UI
    compilerSettingUI = new CompilerSettingUI(this);
    rightLayout->addWidget(compilerSettingUI);

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
    connect(compilerSettingUI, &CompilerSettingUI::buildClicked, this, &MainWindow::onCompile);

    // Output area
    QGroupBox *outputGroup = new QGroupBox(tr("Output"), this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    
    // 编译输出
    outputEdit = new QTextEdit(this);
    outputEdit->setReadOnly(true);
    outputLayout->addWidget(outputEdit);
    
    // 日志面板
    QHBoxLayout *logPanelLayout = new QHBoxLayout();
    logPanelLayout->addWidget(new QLabel(tr("Log:")));
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
        inputEdit->setPlainText(in.readAll());
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
    // 清空 Output 和 Log 窗口的内容
    outputEdit->clear();
    logEdit->clear();

    QString shaderCode = inputEdit->toPlainText();  // 获取 Shader 代码
    QString shaderModel = compilerSettingUI->getShaderModel();
    QString entryPoint = compilerSettingUI->getEntryPoint();
    QString shaderType = compilerSettingUI->getShaderType();
    QString currentCompiler = compilerSettingUI->getCurrentCompiler();  // 获取当前选择的编译器
    QString outputType = compilerSettingUI->getOutputType();  // 获取输出类型

    // 获取当前语言类型
    QString shaderLanguageType = languageCombo->currentText();  // 获取语言类型

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

    // 根据选择的编译器实例化相应的编译器
    if (currentCompiler == "FXC") {
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

        // 调用 fxcCompiler 进行编译
        fxcCompilerInstance->compile(shaderCode, shaderModel, entryPoint, shaderType, includePaths, macros);
    } else if (currentCompiler == "DXC") {
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

        // 调用 dxcCompiler 进行编译
        dxcCompilerInstance->compile(shaderCode, shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    } else if (currentCompiler == "GLSLANG") {
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

        // 调用 glslangCompiler 进行编译
        glslangCompilerInstance->compile(shaderCode, shaderLanguageType, shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    } else if (currentCompiler == "GLSLANGKGVER") {
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

        // 调用 glslangkgverCompiler 进行编译
        glslangkgverCompilerInstance->compile(shaderCode, shaderModel, entryPoint, shaderType, outputType, includePaths, macros);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Unsupported compiler selected."));
    }
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
    onSaveSettings();  // 在析构时保存设置
}

QString MainWindow::settingsFilePath() const
{
    // 使用程序所在目录下的config目录
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + "/config";
    QDir().mkpath(configPath);  // 确保config目录存在
    return configPath + "/settings.ini";  // 返回完整的配置文件路径
}

void MainWindow::loadSettings()
{
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    
    // 恢复窗口几何形状
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    // 恢复窗口位置和大小
    resize(settings.value("windowSize", QSize(800, 600)).toSize());
    move(settings.value("windowPosition", QPoint(100, 100)).toPoint());
    
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

void MainWindow::onSaveSettings()
{
    QSettings settings(settingsFilePath(), QSettings::IniFormat);  // 使用指定路径保存设置
    
    // 保存窗口几何形状
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    
    // 保存窗口位置和大小
    settings.setValue("windowSize", size());
    settings.setValue("windowPosition", pos());
    
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
    
    // 确保所有设置被写入到文件
    settings.sync();
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

// 更新当前编译器设置
void MainWindow::updateCurrentCompilerSettings(const QString &compiler)
{
    // 更新着色器类型、模型和输出类型
    QString shaderType = compilerSettingUI->getShaderType();
    QString shaderModel = compilerSettingUI->getShaderModel();
    QString outputType = compilerSettingUI->getOutputType();

    // 记录设置
    QSettings settings("YourCompany", "YourApp");
    settings.setValue("compiler", compiler);
    settings.setValue("shaderType", shaderType);
    settings.setValue("shaderModel", shaderModel);
    settings.setValue("outputType", outputType);
} 