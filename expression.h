#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <map>
#include <stdexcept>
#include <functional> // 【新增】用于 std::function

// 【新增】引入 Qt 头文件，以便操作 UI
#include <QTextBrowser>
#include <QInputDialog>


//变量表
class EvaluationContext {
public:
    // 定义一个函数类型，用于读取输入
    // 它不接收参数，返回一个 int
    using InputHandler = std::function<int()>;

    // 设置 UI 和 输入处理器
    void setUI(QTextBrowser *out, InputHandler handler) {
        outputBrowser = out;
        inputHandler = handler; // 保存这个“锦囊”函数
    }

    void setValue(std::string var, int value);
    int getValue(std::string var);
    bool isDefined(std::string var);
    void clear();

    void writeOutput(std::string msg) {
        if (outputBrowser) outputBrowser->append(QString::fromStdString(msg));
    }

    // 【修改】现在的 readInput 变得非常简单，它只负责调用“锦囊”
    int readInput(std::string varName) {
        if (!inputHandler) throw std::runtime_error("No input handler defined");
        // 这里会调用 MainWindow 里的那个复杂逻辑，并阻塞直到用户输入完毕
        return inputHandler();
    }

private:
    std::map<std::string, int> symbolTable;
    QTextBrowser *outputBrowser = nullptr;
    InputHandler inputHandler = nullptr; // 【新增】存储外部传入的输入逻辑
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
