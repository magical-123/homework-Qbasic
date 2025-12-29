#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

// 词法分析器
// 职责：将字符串 "10 + A" 切割成 ["10", "+", "A"]
class Tokenizer {
public:
    Tokenizer(std::string input);

    // 获取下一个 Token，如果没有了返回空字符串 ""
    std::string nextToken();

    // 查看下一个 Token 但不消耗它（用于预读）
    std::string peekToken();

    // 检查是否还有 Token
    bool hasMoreTokens();

private:
    std::vector<std::string> tokens; // 存储切分好的所有 Token
    int currentPos; // 当前读到第几个 Token 了

    // 核心函数：执行切割逻辑
    void tokenize(std::string input);
};

#endif // TOKENIZER_H
