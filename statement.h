#ifndef STATEMENT_H
#define STATEMENT_H

#include "expression.h"
#include <string>
#include <stdexcept>

// 定义一个特殊的异常，用于传递 END 信号
class EndSignal : public std::exception {
    // 不需要任何内容，它只是一个信号
};

// 【新增】跳转信号
class GotoSignal : public std::exception {
public:
    int targetLine;
    GotoSignal(int line) : targetLine(line) {}
};

// === 语句基类 ===
class Statement {
public:
    Statement();
    virtual ~Statement();

    // 核心功能：执行这条语句
    // 注意：这里需要传入 context，以便修改变量或读取变量
    // Q: 为什么这还没涉及跳转？
    // A: 简单的语句只修改数据。GOTO 这种控制流我们稍后在 MainWindow 的循环里处理，
    //    或者在这里抛出异常/返回特殊值来触发跳转。为了简单，先定义接口。
    virtual void execute(EvaluationContext &context) = 0;

    // 显示语法树（文档要求的缩进显示）
    // indent: 当前缩进层级
    virtual std::string toString(int indent) = 0;
};

// 1. REM 语句
class RemStmt : public Statement {
public:
    RemStmt(std::string comment);
    virtual void execute(EvaluationContext &context) override;
    virtual std::string toString(int indent) override;
private:
    std::string comment;
};

// 2. LET 语句 (LET var = exp)
class LetStmt : public Statement {
public:
    LetStmt(std::string varName, Expression *exp);
    virtual ~LetStmt();
    virtual void execute(EvaluationContext &context) override;
    virtual std::string toString(int indent) override;
private:
    std::string name;
    Expression *exp;
};

// 3. PRINT 语句 (PRINT exp)
class PrintStmt : public Statement {
public:
    PrintStmt(Expression *exp);
    virtual ~PrintStmt();
    virtual void execute(EvaluationContext &context) override;
    virtual std::string toString(int indent) override;
private:
    Expression *exp;
};

// 4. INPUT 语句 (INPUT var)
class InputStmt : public Statement {
public:
    InputStmt(std::string varName);
    virtual void execute(EvaluationContext &context) override;
    virtual std::string toString(int indent) override;
private:
    std::string name;
};

// 5. END 语句
class EndStmt : public Statement {
public:
    EndStmt();
    virtual void execute(EvaluationContext &context) override;
    virtual std::string toString(int indent) override;
};

// --- GOTO 和 IF 比较特殊，它们需要改变程序执行流 ---
// 我们将在 "execute" 中抛出特殊异常来实现跳转，或者让 execute 返回下一行的行号。
// 这里先定义数据结构。

// 6. GOTO 语句 (GOTO n)
class GotoStmt : public Statement {
public:
    GotoStmt(int lineNumber);
    virtual void execute(EvaluationContext &context) override;
    virtual std::string toString(int indent) override;
    int getLineNumber(); // 特殊访问器
private:
    int lineNumber;
};

// 7. IF 语句 (IF exp1 op exp2 THEN n)
class IfStmt : public Statement {
public:
    IfStmt(Expression *lhs, std::string op, Expression *rhs, int lineNumber);
    virtual ~IfStmt();
    virtual void execute(EvaluationContext &context) override;
    virtual std::string toString(int indent) override;

    // 获取跳转目标和判断条件
    int getLineNumber();
    bool checkCondition(EvaluationContext &context);

private:
    Expression *lhs;
    std::string op; // =, <, >
    Expression *rhs;
    int lineNumber;
};

#endif // STATEMENT_H
