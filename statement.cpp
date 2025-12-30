#include "statement.h"
#include <sstream>

// 辅助函数：生成缩进空格
static std::string indentStr(int n) {
    return std::string(n, ' ');
}

// === Statement ===
Statement::Statement() {}
Statement::~Statement() {}

// === RemStmt ===
RemStmt::RemStmt(std::string comment) : comment(comment) {}
void RemStmt::execute(EvaluationContext &context) { /* do nothing */ }
std::string RemStmt::toString(int indent) {
    return indentStr(indent) + "REM\n" + indentStr(indent + 4) + comment;
}

// === LetStmt ===
LetStmt::LetStmt(std::string varName, Expression *exp) : name(varName), exp(exp) {}
LetStmt::~LetStmt() { delete exp; }

void LetStmt::execute(EvaluationContext &context) {
    int val = exp->eval(context);
    context.setValue(name, val);
}

std::string LetStmt::toString(int indent) {
    std::string str = indentStr(indent) + "LET =\n";
    str += indentStr(indent + 4) + name + "\n";
    // 现在的 exp->toString 会自带换行，并且会基于 indent+4 进行缩进
    str += exp->toString(indent + 4);
    return str;
}

// === PrintStmt ===
PrintStmt::PrintStmt(Expression *exp) : exp(exp) {}
PrintStmt::~PrintStmt() { delete exp; }
void PrintStmt::execute(EvaluationContext &context) {
    int val = exp->eval(context);
    // 调用 Context 的输出能力
    context.writeOutput(std::to_string(val));
}


std::string PrintStmt::toString(int indent) {
    std::string str = indentStr(indent) + "PRINT\n";
    str += exp->toString(indent + 4);
    return str;
}

// === EndStmt ===
EndStmt::EndStmt() {}

void EndStmt::execute(EvaluationContext &context) {
    // 抛出结束信号，打断执行流
    throw EndSignal();
}

std::string EndStmt::toString(int indent) {
    return indentStr(indent) + "END\n";
}

// === InputStmt ===
InputStmt::InputStmt(std::string varName) : name(varName) {}
void InputStmt::execute(EvaluationContext &context) {
    // 调用 Context 的输入能力，并保存变量
    int val = context.readInput(name);
    context.setValue(name, val);
}
std::string InputStmt::toString(int indent) {
    return indentStr(indent) + "INPUT\n" + indentStr(indent + 4) + name;
}

// === GotoStmt ===
GotoStmt::GotoStmt(int lineNumber) : lineNumber(lineNumber) {}
void GotoStmt::execute(EvaluationContext &context) {
    // 抛出跳转信号
    throw GotoSignal(lineNumber);
}
std::string GotoStmt::toString(int indent) {
    return indentStr(indent) + "GOTO\n" + indentStr(indent + 4) + std::to_string(lineNumber);
}
int GotoStmt::getLineNumber() { return lineNumber; }

// === IfStmt ===
IfStmt::IfStmt(Expression *lhs, std::string op, Expression *rhs, int lineNumber)
    : lhs(lhs), op(op), rhs(rhs), lineNumber(lineNumber) {}
IfStmt::~IfStmt() { delete lhs; delete rhs; }

void IfStmt::execute(EvaluationContext &context) {
    // 1. 计算左右表达式
    int l = lhs->eval(context);
    int r = rhs->eval(context);

    // 2. 判断条件
    bool conditionMet = false;
    if (op == "=") conditionMet = (l == r);
    else if (op == "<") conditionMet = (l < r);
    else if (op == ">") conditionMet = (l > r);

    // 3. 如果满足，抛出跳转信号
    if (conditionMet) {
        throw GotoSignal(lineNumber);
    }
    // 如果不满足，什么都不做，程序自然执行下一行
}
bool IfStmt::checkCondition(EvaluationContext &context) {
    int l = lhs->eval(context);
    int r = rhs->eval(context);
    if (op == "=") return l == r;
    if (op == "<") return l < r;
    if (op == ">") return l > r;
    return false;
}
int IfStmt::getLineNumber() { return lineNumber; }

std::string IfStmt::toString(int indent) {
    std::string str = indentStr(indent) + "IF THEN\n";
    str += lhs->toString(indent + 4);
    str += indentStr(indent + 4) + op + "\n";
    str += rhs->toString(indent + 4);
    str += indentStr(indent + 4) + std::to_string(lineNumber) + "\n";
    return str;
}
