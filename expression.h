#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <map>
#include <stdexcept>

// 【新增】引入 Qt 头文件，以便操作 UI
#include <QTextBrowser>
#include <QInputDialog>

// === 1. 上下文环境 (Context) ===
class EvaluationContext {
public:
    // 【新增】设置 UI 组件的指针，在 RUN 开始前调用
    void setUI(QTextBrowser *out, QWidget *parentWidget) {
        outputBrowser = out;
        inputParent = parentWidget;
    }

    void setValue(std::string var, int value);
    int getValue(std::string var);
    bool isDefined(std::string var);
    void clear();

    // 【新增】输出函数 (供 PrintStmt 调用)
    void writeOutput(std::string msg) {
        if (outputBrowser) outputBrowser->append(QString::fromStdString(msg));
    }

    // 【新增】输入函数 (供 InputStmt 调用)
    // 阻塞式弹窗获取输入
    int readInput(std::string varName) {
        bool ok;
        int val = QInputDialog::getInt(inputParent, "INPUT Request",
                                       QString::fromStdString("Value for " + varName + "?"),
                                       0, -2147483647, 2147483647, 1, &ok);
        if (!ok) throw std::runtime_error("Input canceled");
        return val;
    }

private:
    std::map<std::string, int> symbolTable;

    // 【新增】UI 指针
    QTextBrowser *outputBrowser = nullptr;
    QWidget *inputParent = nullptr;
};

// === 2. 表达式基类 (Expression) ===
// 所有的表达式节点（数字、变量、运算）都继承自它
enum ExpressionType { CONSTANT, IDENTIFIER, COMPOUND };

class Expression {
public:
    Expression();
    virtual ~Expression();

    // 核心功能：计算表达式的值
    virtual int eval(EvaluationContext &context) = 0;

    // 核心功能：生成语法树的字符串显示（用于 UI 显示）
    // indent: 缩进层级
    // 比如 CompoundExp 需要先打印 "+" 然后递归打印左右子树
    virtual std::string toString(int indent = 0) = 0;

    virtual ExpressionType type() = 0;

    // 获取优先级的辅助函数（为后续 Parser 准备）
    // 例如 * 比 + 优先级高
    virtual int getConstantValue(); // 仅用于 ConstantExp
    virtual std::string getIdentifierName(); // 仅用于 IdentifierExp
    virtual std::string getOperator(); // 仅用于 CompoundExp
    virtual Expression *getLHS();
    virtual Expression *getRHS();
};

// === 3. 常数表达式 (例如: 10) ===
class ConstantExp : public Expression {
public:
    ConstantExp(int val);

    virtual int eval(EvaluationContext &context) override;
    virtual std::string toString(int indent = 0) override;
    virtual ExpressionType type() override;
    virtual int getConstantValue() override;

private:
    int value;
};

// === 4. 变量表达式 (例如: A) ===
class IdentifierExp : public Expression {
public:
    IdentifierExp(std::string name);

    virtual int eval(EvaluationContext &context) override;
    virtual std::string toString(int indent = 0) override;
    virtual ExpressionType type() override;
    virtual std::string getIdentifierName() override;

private:
    std::string name;
};

// === 5. 复合表达式 (例如: A + 10) ===   表达式树的节点
class CompoundExp : public Expression {
public:
    CompoundExp(std::string op, Expression *lhs, Expression *rhs);
    virtual ~CompoundExp();

    virtual int eval(EvaluationContext &context) override;
    virtual std::string toString(int indent = 0) override;
    virtual ExpressionType type() override;
    virtual std::string getOperator() override;
    virtual Expression *getLHS() override;
    virtual Expression *getRHS() override;

private:
    std::string op;   // 运算符: +, -, *, /, MOD, **
    Expression *lhs;  // 左子树 (Left Hand Side)
    Expression *rhs;  // 右子树 (Right Hand Side)
};

#endif // EXPRESSION_H
