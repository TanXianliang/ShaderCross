#ifndef SHADERCODETEXTEDIT_H
#define SHADERCODETEXTEDIT_H

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QPainter>

class ShaderCodeTextEdit;

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

class LineNumberArea : public QWidget {
public:
    LineNumberArea(ShaderCodeTextEdit* editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    ShaderCodeTextEdit* editor;
};

class ShaderCodeTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit ShaderCodeTextEdit(QWidget *parent = nullptr);
    void setShaderLanguage(const QString &language); // 设置着色器语言

    int lineNumberAreaWidth();

    void resizeEvent(QResizeEvent* event) override;

    void lineNumberAreaPaintEvent(QPaintEvent* event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    }

    void updateLineNumberArea(const QRect& rect, int dy) {
        if (dy) {
            lineNumberArea->scroll(0, dy);
        }
        else {
            lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
        }
    }

private:
    ShaderCodeHighlighter *highlighter;
    LineNumberArea* lineNumberArea;
};

#endif // SHADERCODETEXTEDIT_H
