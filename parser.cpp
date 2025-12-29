#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(std::string line) {
    tokenizer = new Tokenizer(line);
}

Parser::~Parser() {
    delete tokenizer;
}

// 1. 最高层级：处理加减法 (+, -)
// 语法规则: Expression -> Term { (+|-) Term }
Expression* Parser::parseExpression() {
    Expression *lhs = parseTerm();

    while (tokenizer->hasMoreTokens()) {
        std::string token = tokenizer->peekToken();

        if (token == "+" || token == "-") {
            tokenizer->nextToken(); // 消耗掉操作符
            Expression *rhs = parseTerm();
            // 组合成复合表达式，并作为新的左子树（左结合）
            lhs = new CompoundExp(token, lhs, rhs);
        } else {
            break; // 遇到不是加减的符号（比如括号结束），停止
        }
    }
    return lhs;
}

// 2. 中间层级：处理乘除模 (*, /, MOD)
// 语法规则: Term -> Factor { (*|/|MOD) Factor }
Expression* Parser::parseTerm() {
    Expression *lhs = parseFactor();

    while (tokenizer->hasMoreTokens()) {
        std::string token = tokenizer->peekToken();

        if (token == "*" || token == "/" || token == "MOD") {
            tokenizer->nextToken(); // 消耗掉操作符
            Expression *rhs = parseFactor();
            lhs = new CompoundExp(token, lhs, rhs);
        } else {
            break;
        }
    }
    return lhs;
}

// 3. 高级层级：处理幂运算 (**)
// 语法规则: Factor -> Primary [ ** Factor ]
// 注意：幂运算通常是“右结合”的 (2**3**2 = 2**(3**2))
Expression* Parser::parseFactor() {
    Expression *lhs = parsePrimary();

    if (tokenizer->hasMoreTokens()) {
        std::string token = tokenizer->peekToken();

        if (token == "**") {
            tokenizer->nextToken(); // 消耗 **

            // 递归调用 parseFactor 而不是 parsePrimary，实现右结合
            Expression *rhs = parseFactor();

            return new CompoundExp(token, lhs, rhs);
        }
    }
    return lhs;
}

// 4. 最底层：处理数字、变量、括号
// 语法规则: Primary -> Number | Identifier | ( Expression )
Expression* Parser::parsePrimary() {
    std::string token = tokenizer->nextToken();

    if (token == "") throw std::runtime_error("Unexpected end of line");

    // === 情况 A: 数字 ===
    // 简单判断是否为数字（这里简化处理，假设 Tokenizer 切割正确）
    if (isdigit(token[0]) || (token.size()>1 && token[0] == '-' && isdigit(token[1]))) {
        return new ConstantExp(std::stoi(token));
    }

    // === 情况 B: 括号表达式 ===
    if (token == "(") {
        Expression *exp = parseExpression(); // 回到最高层递归

        std::string next = tokenizer->nextToken();
        if (next != ")") {
            delete exp; // 防止内存泄漏
            throw std::runtime_error("Missing closing parenthesis ')'");
        }
        return exp;
    }

    // === 情况 C: 变量 ===
    // 剩下的都当做变量名处理
    return new IdentifierExp(token);
}
