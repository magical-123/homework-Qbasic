#include "tokenizer.h"
#include <cctype> // 用于 isdigit, isalpha, isspace

Tokenizer::Tokenizer(std::string input) {
    currentPos = 0;
    tokenize(input);
}

bool Tokenizer::hasMoreTokens() {
    return currentPos < tokens.size();
}

std::string Tokenizer::nextToken() {
    if (!hasMoreTokens()) return "";
    return tokens[currentPos++];
}

std::string Tokenizer::peekToken() {
    if (!hasMoreTokens()) return "";
    return tokens[currentPos];
}

// 核心切割逻辑
void Tokenizer::tokenize(std::string input) {
    int len = input.length();
    int i = 0;

    while (i < len) {
        char c = input[i];

        // 1. 跳过空白字符 (空格, Tab)
        if (std::isspace(c)) {
            i++;
            continue;
        }

        // 2. 处理数字 (Integers)
        if (std::isdigit(c)) {
            std::string number;
            while (i < len && std::isdigit(input[i])) {
                number += input[i];
                i++;
            }
            tokens.push_back(number);
        }
        // 3. 处理标识符 (Variables 或 关键字如 MOD, LET, IF)
        // 规则：以字母开头，后面可以是字母或数字
        else if (std::isalpha(c)) {
            std::string ident;
            while (i < len && (std::isalnum(input[i]))) {
                ident += input[i];
                i++;
            }
            tokens.push_back(ident);
        }
        // 4. 处理操作符 (Operators)
        else {
            std::string op;
            op += c;

            // 检查是否是双字符操作符 (**, <=, >=, <>)
            if (i + 1 < len) {
                char nextC = input[i + 1];
                bool isDouble = false;

                if (c == '*' && nextC == '*') isDouble = true; // 幂运算 **
                else if (c == '<' && nextC == '=') isDouble = true;
                else if (c == '>' && nextC == '=') isDouble = true;

                if (isDouble) {
                    op += nextC;
                    i++; // 多跳过一个字符
                }
            }

            tokens.push_back(op);
            i++;
        }
    }
}
