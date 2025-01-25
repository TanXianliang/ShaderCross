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

class CompilerSettingUI : public QWidget
{
    Q_OBJECT

public:
    explicit CompilerSettingUI(QWidget *parent = nullptr);

    // 获取当前设置
    QString getCurrentCompiler() const;
    QString getShaderType() const;
    QString getEntryPoint() const;
    QString getShaderModel() const;
    QString getOutputType() const;

    // 设置当前配置
    void setCurrentCompiler(const QString &compiler);
    void setShaderType(const QString &type);
    void setEntryPoint(const QString &entry);
    void setShaderModel(const QString &model);
    void setOutputType(const QString &type);

public slots:
    void onLanguageChanged(const QString &language);
    void updateCompilerSettings(const QString &compiler);

signals:
    void buildClicked();
    void compilerChanged(const QString &compiler);

private:
    // UI 组件
    QComboBox *compilerCombo;
    QComboBox *shaderTypeCombo;
    QLineEdit *entryPointEdit;
    QComboBox *shaderModelCombo;
    QComboBox *outputTypeCombo;
    QPushButton *buildButton;

    void setupUI();
    void setupConnections();
};

#endif // COMPILERSETTINGUI_H
