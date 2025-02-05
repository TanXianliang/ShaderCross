#ifndef SHADERCODETEXTEDIT_H
#define SHADERCODETEXTEDIT_H

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>

class ShaderCodeHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    ShaderCodeHighlighter(QTextDocument *parent = nullptr);

    QStringList keywords;

protected:
    void highlightBlock(const QString &text) override;

private:
    void highlightKeyword(const QString &text);
    void highlightComment(const QString &text);
    void highlightString(const QString &text);
};

class ShaderCodeTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit ShaderCodeTextEdit(QWidget *parent = nullptr);
    void setShaderLanguage(const QString &language); // 设置着色器语言

private:
    ShaderCodeHighlighter *highlighter;
};

#endif // SHADERCODETEXTEDIT_H
