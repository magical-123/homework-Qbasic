#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <map>
#include <stdexcept>

// === 1. 上下文环境 (Context) ===
// 用来存储变量的值，比如 A=10, B=20
class EvaluationContext {
public:
    void setValue(std::string var, int value);
    int getValue(std::string var);
    bool isDefined(std::string var);
    void clear(); // 清空所有变量

private:
    std::map<std::string, int> symbolTable;
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
    virtual std::string toString() = 0;

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
    virtual std::string toString() override;
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
    virtual std::string toString() override;
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
    virtual std::string toString() override;
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
