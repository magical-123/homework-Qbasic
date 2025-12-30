#include "mainwindow.h"
#include "expression.h"
#include "tokenizer.h"
#include "parser.h"
#include "ui_mainwindow.h"
#include "statement.h"
#include <QFileDialog> // 用于打开文件
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 【新增】初始化 Context 的 UI 接口
    // 这样无论是 RUN 还是立即执行，PRINT 都能打印到窗口上
    globalContext.setUI(ui->textBrowser, this);

    //注意不要重复链接，已经自动链接了。
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 核心逻辑：处理用户输入
void MainWindow::on_cmdLineEdit_editingFinished()
{
    QString cmd = ui->cmdLineEdit->text().trimmed(); // 去除首尾空格
    ui->cmdLineEdit->setText(""); // 清空输入框

    if (cmd.isEmpty()) return;

    // 在结果窗口回显用户的输入（模拟终端风格）
    // ui->textBrowser->append(cmd); // 如果不想回显可以注释掉这行

    // 1. 尝试读取行号
    bool isNumber;
    // 获取第一个单词，看看是不是数字
    QString firstToken = cmd.section(' ', 0, 0);
    int lineNumber = firstToken.toInt(&isNumber);

    if (isNumber) {
        // === 情况 A: 用户输入了行号 (例如 "10 LET A = 1") ===

        // 获取行号后面的内容
        // mid(firstToken.length()) 截取掉行号，trimmed() 去掉剩下的前导空格
        QString codeContent = cmd.mid(firstToken.length()).trimmed();

        if (codeContent.isEmpty()) {
            // 如果只输入了行号 (例如 "10") -> 删除该行
            programCode.erase(lineNumber);
        } else {
            // 否则 -> 插入或更新该行
            programCode[lineNumber] = codeContent;
        }

        // 数据变了，刷新显示
        refreshCodeDisplay();

    } else {
        // === 情况 B: 用户输入的是命令 (例如 "RUN", "LOAD") ===

        if (cmd.compare("LOAD", Qt::CaseInsensitive) == 0) {
            on_btnLoadCode_clicked();
        }
        else if (cmd.compare("CLEAR", Qt::CaseInsensitive) == 0) {
            on_btnClearCode_clicked();
        }
        else if (cmd.compare("QUIT", Qt::CaseInsensitive) == 0) {
            QApplication::quit();
        }
        else if (cmd.compare("HELP", Qt::CaseInsensitive) == 0) {
            ui->textBrowser->append("QBasic Interpreter Help:\nType line numbers to add code.\nType RUN to execute.");
        }
        else {
            // 暂时不处理其他立即执行的语句（如 PRINT 1+1），留给后续阶段
            ui->textBrowser->append("Error: Unknown command or immediate execution not implemented yet.");
        }
    }
}

// 辅助函数：遍历 map 更新 UI
void MainWindow::refreshCodeDisplay()
{
    ui->CodeDisplay->clear();
    // 遍历 map，因为它自动按 Key (行号) 排序
    for (auto it = programCode.begin(); it != programCode.end(); ++it) {
        // 拼接格式： "10 LET A = 1"
        QString lineStr = QString::number(it->first) + " " + it->second;
        ui->CodeDisplay->append(lineStr);
    }
}

// 实现 CLEAR 功能
void MainWindow::on_btnClearCode_clicked()
{
    programCode.clear();
    ui->CodeDisplay->clear();
    ui->textBrowser->clear();
    ui->treeDisplay->clear();
}

// 实现 LOAD 功能
void MainWindow::on_btnLoadCode_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Basic File"), "", tr("Text Files (*.txt)"));

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    // 清空当前代码（或者你可以选择保留，看需求）
    programCode.clear();

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 解析每一行文件
        bool isNumber;
        QString firstToken = line.section(' ', 0, 0);
        int lineNumber = firstToken.toInt(&isNumber);

        if (isNumber) {
            QString codeContent = line.mid(firstToken.length()).trimmed();
            if (!codeContent.isEmpty()) {
                programCode[lineNumber] = codeContent;
            }
        }
    }

    refreshCodeDisplay();
    ui->textBrowser->append("Loaded: " + fileName);
}
// Run 按钮的槽函数
void MainWindow::on_btnRunCode_clicked()
{
    ui->treeDisplay->clear();
    ui->textBrowser->clear();

    if (programCode.empty()) return;

    // === 1. 解析阶段 (Parsing Phase) ===
    // 我们需要把所有代码先解析成 Statement 对象，存储起来方便跳转
    std::map<int, Statement*> statementMap;
    EvaluationContext context;
    context.setUI(ui->textBrowser, this); // 把 UI 传给 Context

    try {
        for (auto it = programCode.begin(); it != programCode.end(); ++it) {
            int lineNum = it->first;
            QString code = it->second;

            // 调用 Parser
            Parser parser(code.toStdString());
            Statement *stmt = parser.parseStatement();

            // 存入 map
            statementMap[lineNum] = stmt;

            // 显示语法树
            // 1. 获取缩进为 0 的树字符串
            std::string treeStrStd = stmt->toString(0);
            QString treeStr = QString::fromStdString(treeStrStd);

            // 2. 去掉末尾可能多余的换行符 (防止 append 多次换行)
            if (treeStr.endsWith('\n')) treeStr.chop(1);

            // 3. 拼接 "行号" + "空格" + "树结构"
            // 结果类似: "100 REM" 或 "110 LET ="
            ui->treeDisplay->append(QString::number(lineNum) + " " + treeStr);
        }
    } catch (std::exception &e) {
        ui->textBrowser->append("Syntax Error: " + QString::fromStdString(e.what()));
        // 如果解析出错，记得释放已经创建的语句内存
        for (auto pair : statementMap) delete pair.second;
        return;
    }

    // === 2. 执行阶段 (Execution Phase) ===
    try {
        // 从第一行开始
        auto it = statementMap.begin();

        while (it != statementMap.end()) {
            Statement *currentStmt = it->second;

            try {
                // 执行语句
                currentStmt->execute(context);

                // 正常执行完，移动到下一行
                it++;
            }
            catch (GotoSignal &gotoSig) {
                // 捕获 GOTO 信号
                int targetLine = gotoSig.targetLine;

                // 在 map 中查找目标行
                auto targetIt = statementMap.find(targetLine);
                if (targetIt == statementMap.end()) {
                    throw std::runtime_error("Line number not found: " + std::to_string(targetLine));
                }

                // 跳转迭代器
                it = targetIt;
            }
        }
    }
    catch (EndSignal &endSig) {
        // 正常结束 (END 语句)
        // ui->textBrowser->append("--- Program Ended ---");
    }
    catch (std::exception &e) {
        // 运行时错误 (如除以0，变量未定义)
        ui->textBrowser->append("Runtime Error: " + QString::fromStdString(e.what()));
    }

    // === 3. 清理内存 ===
    for (auto pair : statementMap) {
        delete pair.second;
    }
}
