#ifndef PARSER_H
#define PARSER_H

#include "expression.h"
#include "tokenizer.h"
#include "statement.h"
#include <string>

class Parser {
public:
    Parser(std::string line);
    ~Parser();

    // 主入口：解析并返回表达式树的根节点
    // 调用者负责 delete 返回的指针
    Expression* parseExpression();
    Statement* parseStatement();

private:
    Tokenizer *tokenizer; // 词法分析器实例

    // 递归下降子函数
    Expression* parseTerm();     // 处理 *, /, MOD
    Expression* parseFactor();   // 处理 **
    Expression* parsePrimary();  // 处理 (), 数字, 变量
};

#endif // PARSER_H
