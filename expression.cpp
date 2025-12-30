#include "expression.h"
#include <cmath>    // std::pow
#include <string>
#include <stdexcept> // std::runtime_error
#include <sstream>

// 生成 n 个空格
static std::string indentStr(int n) {
    return std::string(n, ' ');
}

// ==========================================================
// EvaluationContext (变量上下文) 实现
// ==========================================================

void EvaluationContext::setValue(std::string var, int value) {
    symbolTable[var] = value;
}

int EvaluationContext::getValue(std::string var) {
    if (isDefined(var)) {
        return symbolTable[var];
    }
    return 0; // BASIC 默认未初始化的变量为 0
}

bool EvaluationContext::isDefined(std::string var) {
    return symbolTable.find(var) != symbolTable.end();
}

void EvaluationContext::clear() {
    symbolTable.clear();
}

// ==========================================================
// Expression (基类) 默认实现
// ==========================================================

Expression::Expression() {}
Expression::~Expression() {}

std::string Expression::getIdentifierName() { return ""; }
std::string Expression::getOperator() { return ""; }
int Expression::getConstantValue() { return 0; }
Expression* Expression::getLHS() { return nullptr; }
Expression* Expression::getRHS() { return nullptr; }

// ==========================================================
// ConstantExp (常数) 实现
// ==========================================================

ConstantExp::ConstantExp(int val) : value(val) {}

int ConstantExp::eval(EvaluationContext &context) {
    return value;
}

std::string ConstantExp::toString(int indent) {
    // 缩进 + 数值 + 换行
    return indentStr(indent) + std::to_string(value) + "\n";
}

ExpressionType ConstantExp::type() {
    return CONSTANT;
}

int ConstantExp::getConstantValue() {
    return value;
}

// ==========================================================
// IdentifierExp (变量) 实现
// ==========================================================

IdentifierExp::IdentifierExp(std::string name) : name(name) {}

int IdentifierExp::eval(EvaluationContext &context) {
    if (!context.isDefined(name)) {
        // 可以在这里抛出异常，或者静默返回0 (符合Minimal Basic特性)
        // throw std::runtime_error("Variable '" + name + "' is not defined");
        return 0;
    }
    return context.getValue(name);
}

std::string IdentifierExp::toString(int indent) {
    // 缩进 + 变量名 + 换行
    return indentStr(indent) + name + "\n";
}

ExpressionType IdentifierExp::type() {
    return IDENTIFIER;
}

std::string IdentifierExp::getIdentifierName() {
    return name;
}

// ==========================================================
// CompoundExp (复合运算) 实现
// ==========================================================

CompoundExp::CompoundExp(std::string op, Expression *lhs, Expression *rhs)
    : op(op), lhs(lhs), rhs(rhs) {}

CompoundExp::~CompoundExp() {
    delete lhs; // 递归删除左子树
    delete rhs; // 递归删除右子树
}

int CompoundExp::eval(EvaluationContext &context) {
    int leftVal = lhs->eval(context);
    int rightVal = rhs->eval(context);

    if (op == "+") return leftVal + rightVal;
    if (op == "-") return leftVal - rightVal;
    if (op == "*") return leftVal * rightVal;

    if (op == "/") {
        if (rightVal == 0) throw std::runtime_error("Division by zero");
        return leftVal / rightVal;
    }

    if (op == "MOD") {
        if (rightVal == 0) throw std::runtime_error("Division by zero");
        // 题目要求：r 的符号与 b (rightVal) 相同
        int r = leftVal % rightVal;
        if ((rightVal > 0 && r < 0) || (rightVal < 0 && r > 0)) {
            r += rightVal;
        }
        return r;
    }

    if (op == "**") {
        return (int)std::pow(leftVal, rightVal);
    }

    throw std::runtime_error("Illegal operator: " + op);
}

std::string CompoundExp::toString(int indent) {
    std::string str;
    // 1. 打印操作符 (根)
    str += indentStr(indent) + op + "\n";
    // 2. 递归打印左子树 (缩进 + 4)
    str += lhs->toString(indent + 4);
    // 3. 递归打印右子树 (缩进 + 4)
    str += rhs->toString(indent + 4);
    return str;
}

ExpressionType CompoundExp::type() {
    return COMPOUND;
}

std::string CompoundExp::getOperator() {
    return op;
}

Expression* CompoundExp::getLHS() {
    return lhs;
}

Expression* CompoundExp::getRHS() {
    return rhs;
}
