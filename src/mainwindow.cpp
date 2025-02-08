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
#include <QtWidgets/QToolButton>
#include <QtWidgets/QMdiArea>
#include <QTabWidget>
#include <QDebug>
#include <QtGui/QPainter>
#include <QtGui/QScreen>
#include <QtGui/QGuiApplication>
 #include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isDarkTheme(true)
    , resizing(false)
    , currentDocument(nullptr)
{
    // 修改窗口标志，添加系统菜单和最小化最大化按钮的支持
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | 
                  Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    //setAttribute(Qt::WA_TranslucentBackground);
    //setWindowOpacity(1);

    // 设置程序图标
    setWindowIcon(QIcon(":/resources/icons/icons.jpg"));
    
    // 设置窗口标题
    setWindowTitle("ShaderCross");

    setupUI();
    createMenus();
    applyTheme(isDarkTheme);
    loadSettings();  // 加载上次的设置

    closeButton = new QToolButton(this);
    minButton = new QToolButton(this);
    maxButton = new QToolButton(this);  // 新增最大化按钮

    QPixmap *closePix = new QPixmap(":/resources/icons/closeicon.png");
    QPixmap *minPix = new QPixmap(":/resources/icons/minicon.png");
    QPixmap *maxPix = new QPixmap(":/resources/icons/maxicon.png");  // 最大化图标
    QPixmap *returnPix = new QPixmap(":/resources/icons/returnicon.png");  // 还原图标

    closeButton->setIcon(*closePix);
    minButton->setIcon(*minPix);
    maxButton->setIcon(*maxPix);  // 默认显示最大化图标

    updateButtonPositions();  // 初始化按钮位置

    closeButton->setToolTip(tr("close"));
    minButton->setToolTip(tr("minimum"));
    maxButton->setToolTip(tr("maximize"));  // 设置提示文本
    
    connect(closeButton, &QToolButton::clicked, this, &MainWindow::windowclosed);
    connect(minButton, &QToolButton::clicked, this, &MainWindow::windowmin);
    connect(maxButton, &QToolButton::clicked, this, [=]() {  // 最大化/还原切换
        if (isMaximized()) {
            showNormal();
            maxButton->setIcon(*maxPix);
            maxButton->setToolTip(tr("maximize"));
        } else {
            showMaximized();
            maxButton->setIcon(*returnPix);
            maxButton->setToolTip(tr("restore"));
        }
    });

    // 启用鼠标追踪
    setMouseTracking(true);
    centralWidget()->setMouseTracking(true);

    qApp->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    if (currentDocument) {
        delete currentDocument;
        currentDocument = nullptr;
    };

    saveSettings();  // 在析构时保存设置
}

//为自定义的关闭及最小化提供实际功能
void MainWindow::windowclosed()
{
    this->close();
}

void MainWindow::windowmin()
{
    this->showMinimized();
}

void MainWindow::setupUI()
{
    tabWidget = new QTabWidget(this); // 初始化 Tab 控件
    setCentralWidget(tabWidget); // 设置为主窗口的中心部件

    tabWidget->tabBar()->setTabsClosable(true); // 启用关闭按钮
    tabWidget->tabBar()->setMovable(true); // 允许拖动标签

    //connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    connect(tabWidget->tabBar(), SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
}

void MainWindow::createMenus()
{
    QMenuBar* bar = menuBar();
    bar->installEventFilter(this);
    
    // 设置菜单栏可以拖动窗口
    connect(bar, &QMenuBar::customContextMenuRequested, [=](const QPoint &pos) {
        Q_UNUSED(pos);
    });
    
    QMenu *fileMenu = bar->addMenu(tr("File"));
    fileMenu->addAction(tr("New Document"), this, &MainWindow::onNewDocument, QKeySequence::New);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Open Workspace"), this, &MainWindow::onNewDocumentByOpenWorkspace);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Save Code"), this, &MainWindow::onSaveResult, QKeySequence::Save);
    fileMenu->addAction(tr("Save Workspace"), this, &MainWindow::onSaveWorkspace);
    
    QMenu *editMenu = bar->addMenu(tr("Edit"));
    editMenu->addAction(tr("Undo"), this, [this](){ if (currentDocument) currentDocument->undo(); }, QKeySequence::Undo);
    editMenu->addAction(tr("Redo"), this, [this](){ if (currentDocument) currentDocument->redo(); }, QKeySequence::Redo);
    
    QMenu *buildMenu = bar->addMenu(tr("Build"));
    buildMenu->addAction(tr("Compile"), this, [this](){ if (currentDocument) currentDocument->compile();}, Qt::Key_F5);

    QMenu *uiMenu = bar->addMenu(tr("UI"));
    uiMenu->addAction(tr("Reset Layout"), this, &MainWindow::onResetLayout);
    
    uiMenu->addSeparator();
    uiMenu->addAction(tr("Toggle Theme"), this, &MainWindow::onToggleTheme, QKeySequence(Qt::CTRL | Qt::Key_T));
    
    // 设置菜单栏鼠标事件追踪
    bar->setMouseTracking(true);
}

// 事件过滤器，用于处理菜单栏和窗口的鼠标事件
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 检查是否是菜单栏的事件
    if (obj == menuBar()) {
        static bool isDoubleClick = false; // 记录是否双击
        static bool isPressOnButton = false; // 记录是否按下按钮

        switch (event->type()) {
            case QEvent::MouseButtonPress: { // 鼠标按下事件
                auto menuBarInstance = menuBar();
                int menuBarMaxWidth = 0; // 菜单栏最大宽度

                // 遍历菜单中的所有动作
                for (QAction *action : menuBarInstance->actions()) {
                    // 获取动作的矩形区域
                    QRect actionRect = menuBarInstance->actionGeometry(action); 
                    if (actionRect.right() > menuBarMaxWidth) {
                        menuBarMaxWidth = actionRect.right(); // 更新最大宽度
                    }
                }

                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                auto mousePosX = mouseEvent->globalPos().x() - this->pos().x();
                if (mousePosX < menuBarMaxWidth) {
                    isPressOnButton = true; // 按下按钮
                }

                if (mouseEvent->button() == Qt::LeftButton) {
                    mouseQPoint = mouseEvent->globalPos() - this->pos(); // 记录鼠标位置
                }
                break;
            }

            case QEvent::MouseButtonDblClick: { // 鼠标双击事件
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    isDoubleClick = true; // 设置双击标志
                    if (isMaximized()) { // 如果窗口已最大化
                        showNormal(); // 还原窗口
                        maxButton->setIcon(QIcon(":/resources/icons/maxicon.png")); // 设置最大化图标
                        maxButton->setToolTip(tr("maximize")); // 设置提示信息
                        return true;
                    } else {
                        showMaximized(); // 最大化窗口
                        maxButton->setIcon(QIcon(":/resources/icons/returnicon.png")); // 设置还原图标
                        maxButton->setToolTip(tr("restore")); // 设置提示信息
                        return true;
                    }
                }
                break;
            }

            case QEvent::MouseMove: { // 鼠标移动事件
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (!isPressOnButton && !isDoubleClick && (mouseEvent->buttons() & Qt::LeftButton)) {
                    if (isMaximized()) { // 如果窗口已最大化
                        // 在最大化状态下拖动时还原窗口
                        QRect geo = normalGeometry();
                        showNormal();

                        maxButton->setIcon(QIcon(":/resources/icons/maxicon.png")); // 设置最大化图标
                        maxButton->setToolTip(tr("maximize")); // 设置提示信息

                        geo.moveLeft(mouseEvent->globalPos().x() - geo.width() / 2);
                        geo.moveTop(mouseEvent->globalPos().y());

                        setGeometry(geo); // 设置窗口几何形状

                        mouseQPoint = mouseEvent->globalPos() - this->pos(); // 更新鼠标位置
                    }
                    else
                    {
                        move(mouseEvent->globalPos() - mouseQPoint); // 移动窗口
                    }
                }
                break;
            }

            case QEvent::MouseButtonRelease: { // 鼠标释放事件
                isDoubleClick = false; // 重置双击标志
                isPressOnButton = false; // 重置按钮按下标志
                break;
            }
        }
    }

    // 检查是否是当前窗口的鼠标按下事件
    if (obj == this && !resizing && !isMaximized() && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // 检查鼠标位置以确定是否开始调整大小
            if (mouseEvent->pos().y() >= height() - RESIZE_MARGIN) {
                resizing = true; // 开始调整大小
                resizeEdge = Qt::BottomEdge; // 设置调整边缘为底部
            }
            else if (mouseEvent->pos().x() <= RESIZE_MARGIN) {
                resizing = true; // 开始调整大小
                resizeEdge = Qt::LeftEdge; // 设置调整边缘为左侧
            }
            else if (mouseEvent->pos().x() >= width() - RESIZE_MARGIN) {
                resizing = true; // 开始调整大小
                resizeEdge = Qt::RightEdge; // 设置调整边缘为右侧
            }
            mouseQPoint = mouseEvent->globalPos(); // 记录鼠标位置
        }
    }

    // 处理鼠标移动事件
    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if (resizing && (mouseEvent->buttons() & Qt::LeftButton)) { // 如果正在调整大小
            QPoint delta = mouseEvent->globalPos() - mouseQPoint; // 计算鼠标移动的增量
            QRect geo = geometry(); // 获取当前窗口几何形状

            // 根据调整边缘更新窗口几何形状
            if (resizeEdge & Qt::LeftEdge) {
                geo.setLeft(geo.left() + delta.x());
            }
            if (resizeEdge & Qt::RightEdge) {
                geo.setRight(geo.right() + delta.x());
            }
            if (resizeEdge & Qt::TopEdge) {
                geo.setTop(geo.top() + delta.y());
            }
            if (resizeEdge & Qt::BottomEdge) {
                geo.setBottom(geo.bottom() + delta.y());
            }

            setGeometry(geo); // 设置新的窗口几何形状
            mouseQPoint = mouseEvent->globalPos(); // 更新鼠标位置
        }

        // 更新鼠标样式
        if (obj == this) {
            if (mouseEvent->pos().y() <= RESIZE_MARGIN || mouseEvent->pos().y() >= height() - RESIZE_MARGIN) {
                setCursor(Qt::SizeVerCursor); // 设置为垂直调整大小光标
            }
            else if (mouseEvent->pos().x() <= RESIZE_MARGIN || mouseEvent->pos().x() >= width() - RESIZE_MARGIN) {
                setCursor(Qt::SizeHorCursor); // 设置为水平调整大小光标
            }
            else {
                setCursor(Qt::ArrowCursor); // 设置为箭头光标
            }
        }
        else if (!resizing) {
            setCursor(Qt::ArrowCursor); // 设置为箭头光标
        }
    }

    // 鼠标释放事件
    if (event->type() == QEvent::MouseButtonRelease)
    {
        resizing = false; // 结束调整大小
        resizeEdge = Qt::Edge(); // 重置调整边缘
    }

    return QMainWindow::eventFilter(obj, event); // 调用基类的事件过滤器
}

void MainWindow::onNewDocument()
{
    QString documentName;
    bool ok = false;
    static int documentIndex = 0;

    do {
        // 弹出对话框输入文档名称
        documentName = QInputDialog::getText(this, QString("Document Naming"), QString("Name:"), QLineEdit::Normal, QString("untitled-%1").arg(documentIndex), &ok);
        if (!ok || documentName.isEmpty()) {
            return; // 如果用户取消或输入为空，则返回
        }
        documentIndex++; // 递增索引

        // 检查是否存在同名文档
        for (int i = 0; i < tabWidget->count(); ++i) {
            if (tabWidget->tabText(i).toLower() == documentName.toLower()) {
                ok = false;
                break;
            }
        }

        if (!ok) {
            QMessageBox::warning(this, QString("Warnings"), QString("The document name already exists, please rename it."));
        }
    } while (!ok);

    // 创建新的 DocumentWindow 实例
    documentName = documentName.toLower();
    DocumentWindow *newDocument = new DocumentWindow(this, documentName);
    
    // 将新文档添加到 Tab 控件
    int tabIndex = tabWidget->addTab(newDocument, documentName);
    tabWidget->tabBar()->setTabsClosable(true); // 启用关闭按钮
    newDocument->show(); // 显示新文档窗口
}

void MainWindow::onNewDocumentByOpenWorkspace()
{
    QString documentName;
    bool ok = false;
    static int documentIndex = 0;

    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Workspace Setting"),
        QString(),
        tr("Workspace Setting (*.ini);;All Files (*.*)")
    );

    if (!filePath.isEmpty()) {
        do {
            // 弹出对话框输入文档名称
            documentName = QInputDialog::getText(this, QString("Document Naming"), QString("Name:"), QLineEdit::Normal, QString("untitled-%1").arg(documentIndex), &ok);
            if (!ok || documentName.isEmpty()) {
                return; // 如果用户取消或输入为空，则返回
            }
            documentIndex++; // 递增索引

            // 检查是否存在同名文档
            for (int i = 0; i < tabWidget->count(); ++i) {
                if (tabWidget->tabText(i).toLower() == documentName.toLower()) {
                    ok = false;
                    break;
                }
            }

            if (!ok) {
                QMessageBox::warning(this, QString("Warnings"), QString("The document name already exists, please rename it."));
            }

            // 创建新的 DocumentWindow 实例
            documentName = documentName.toLower();
            DocumentWindow* newDocument = new DocumentWindow(this, documentName, filePath);

            // 将新文档添加到 Tab 控件
            int tabIndex = tabWidget->addTab(newDocument, documentName);
            tabWidget->tabBar()->setTabsClosable(true); // 启用关闭按钮
            newDocument->show(); // 显示新文档窗口
        } while (!ok);
    }
}

void MainWindow::onSaveResult()
{
    QWidget* tab = tabWidget->currentWidget(); // 获取当前选中的标签页
    if (tab) {
        // 获取tab的documentWindow
        DocumentWindow* documentWindow = dynamic_cast<DocumentWindow*>(tab); // 将当前标签页转换为DocumentWindow类型
        if (documentWindow) {
            QString title = documentWindow->getDocumentWindowTitle(); // 获取文档标题
            QString language = documentWindow->getLanguage(); // 获取文档语言

            QString defaultFilePath = title + "." + language; // 生成默认文件路径
            defaultFilePath = defaultFilePath.toLower(); // 转换为小写

            QString fileName = QFileDialog::getSaveFileName(this, tr("Save Code"), defaultFilePath, tr("All Files (*.*)")); // 打开保存文件对话框
            if (!fileName.isEmpty()) { // 如果用户选择了文件名
                auto content = documentWindow->getContent(); // 获取文档内容
                QFile file(fileName); // 创建文件对象
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) { // 尝试打开文件进行写入
                    QTextStream out(&file); // 创建文本流

                    // 根据文档编码设置文本流编码
                    if (documentWindow->getEncoding() == "UTF-8") {
                        out.setCodec("UTF-8");
                    }
                    else if (documentWindow->getEncoding() == "GB2312") {
                        out.setCodec("GB2312");
                    }
                    else if (documentWindow->getEncoding() == "GBK") {
                        out.setCodec("GBK");
                    }

                    out << content; // 将内容写入文件
                    file.close(); // 关闭文件
                }
                else {
                    QMessageBox::warning(this, QString("Warnings"), QString("Saving file failed.")); // 显示保存失败的警告
                }
            }
        }
    }
}

void MainWindow::onSaveWorkspace()
{
    QWidget* tab = tabWidget->currentWidget(); // 获取当前选中的标签页
    if (tab) {
        // 获取tab的documentWindow
        DocumentWindow* documentWindow = dynamic_cast<DocumentWindow*>(tab); // 将当前标签页转换为DocumentWindow类型
        QString defaultFilePath = documentWindow->getDocumentWindowTitle() + ".ini"; // 生成默认工作区文件路径
        defaultFilePath = defaultFilePath.toLower(); // 转换为小写

        if (documentWindow) {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save Workspace"), defaultFilePath, tr("Setting Files (*.ini)")); // 打开保存工作区对话框
            if (!fileName.isEmpty()) { // 如果用户选择了文件名
                documentWindow->saveSettings(fileName); // 保存文档窗口的设置
            }
        }
    }
}

void MainWindow::onShowDisassembly()
{
    QMessageBox::information(this, tr("Notice"), tr("Disassembly not implemented yet"));
}

void MainWindow::onResetLayout()
{
    resize(1920, 1080);
    // TODO: 重置其他布局设置
}

void MainWindow::onToggleTheme()
{
    isDarkTheme = !isDarkTheme;
    applyTheme(isDarkTheme);
}

void MainWindow::applyTheme(bool dark)
{
    isDarkTheme = dark;
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

            /* QTabWidget 样式 */
            QTabWidget {
                background-color: #252526;
                border: 1px solid #2d2d2d;
            }

            QTabWidget::pane {
                background-color: #252526;
                border: 1px solid 2d2d2d;
            }

            QTabBar {
                background-color: #252526;
                color: #d4d4d4;
            }

            QTabBar::tab {
                background: #3c3c3c;
                padding: 10px;
                border: 1px solid #2d2d2d;
                border-bottom: none;
            }

            QTabBar::tab:selected {
                background: #1e1e1e;
                color: #ffffff;
            }

            QTabBar::tab:hover {
                background: #4a6c94; /* 鼠标悬停时背景色 */
            }

           QTabBar::close-button {
                image: url(:/resources/icons/closeicon.png); /* 设置关闭图标 */
            }
        )";
        setStyleSheet(darkStyle);

        if (currentDocument) {
            currentDocument->setStyleSheet(darkStyle);
        }
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

            /* QTabWidget 样式 */
            QTabWidget {
                background-color: #ffffff;
                border: 1px solid #e0e0e0;
            }

            QTabBar {
                background-color: #f5f5f5;
                color: #000000;
            }

            QTabBar::tab {
                background: #e0e0e0;
                padding: 10px;
                border: 1px solid #e0e0e0;
                border-bottom: none;
            }

            QTabBar::tab:selected {
                background: #ffffff;
                color: #000000;
            }

            QTabBar::tab:hover {
                background: #d8d8d8;
            }

            QTabBar::close-button {
                image: url(:/resources/icons/closeicon.png); /* 设置关闭图标 */
            }
        )";
        setStyleSheet(lightStyle);
        if (currentDocument) {
            currentDocument->setStyleSheet(lightStyle);
        }
    }
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

    // 自动恢复所有文档
    QDir tempDocsDir(QCoreApplication::applicationDirPath() + "/config/temp_docs");
    QStringList iniFiles = tempDocsDir.entryList(QStringList() << "*.ini", QDir::Files);
    
    for (const QString &fileName : iniFiles) {
        // 去除扩展名，只保留文件名
        QString documentName = fileName.left(fileName.lastIndexOf('.'));
        documentName = documentName.toLower();

        DocumentWindow *document = new DocumentWindow(this, documentName);
        tabWidget->addTab(document, documentName);
    }
}

void MainWindow::saveSettings()
{
    QSettings settings(settingsFilePath(), QSettings::IniFormat);  // 使用指定路径保存设置
    
    // 保存窗口几何形状
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    
    // 保存窗口位置和大小
    settings.setValue("windowSize", size());
    settings.setValue("windowPosition", pos());
    
    // 确保所有设置被写入到文件
    settings.sync();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateButtonPositions();
}

void MainWindow::updateButtonPositions()
{
    if (closeButton && minButton && maxButton) {
        closeButton->setGeometry(frameGeometry().width() - 35, 5, 30, 30);
        maxButton->setGeometry(frameGeometry().width() - 65, 5, 30, 30);  // 最大化按钮
        minButton->setGeometry(frameGeometry().width() - 95, 5, 30, 30);  // 调整最小化按钮位置
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::onTabCloseRequested(int index)
{ 
    QWidget *tab = tabWidget->widget(index);
    if (tab) {
        // 获取tab的documentWindow
        DocumentWindow *documentWindow = dynamic_cast<DocumentWindow*>(tab);
        if (documentWindow) {
            documentWindow->enableSave(false);
        }
        documentWindow = nullptr;

        tabWidget->removeTab(index);
        delete tab; // 释放内存
    }
}

void MainWindow::onTabDoubleClicked(int index)
{
    if (index >= 0) { // 确保索引有效
        tabWidget->removeTab(index); // 移除标签
    }
} 