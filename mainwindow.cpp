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

    // 【核心改动】配置 Context，注入输入逻辑
    // 使用 lambda 表达式包裹我们的 handleInputFromCommandLine
    globalContext.setUI(ui->textBrowser, [this]() -> int {
        return this->handleInputFromCommandLine();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

// =========================================================
// 2. 命令行输入处理逻辑 (包含立即执行)
// =========================================================
void MainWindow::on_cmdLineEdit_editingFinished()
{
    QString cmd = ui->cmdLineEdit->text().trimmed();
    ui->cmdLineEdit->setText(""); // 清空输入框

    if (cmd.isEmpty()) return;

    // 尝试解析行号
    bool isNumber;
    QString firstToken = cmd.section(' ', 0, 0);
    int lineNumber = firstToken.toInt(&isNumber);

    if (isNumber) {
        // === 情况 A: 编辑代码行 (有行号) ===
        // 格式: 10 LET A = 1
        QString codeContent = cmd.mid(firstToken.length()).trimmed();

        if (codeContent.isEmpty()) {
            // 输入 "10" -> 删除第10行
            programCode.erase(lineNumber);
        } else {
            // 输入 "10 ..." -> 插入或更新
            programCode[lineNumber] = codeContent;
        }
        refreshCodeDisplay();
    }
    else {
        // === 情况 B: 系统命令 (无行号) ===
        if (cmd.compare("RUN", Qt::CaseInsensitive) == 0) {
            on_btnRunCode_clicked();
            return;
        }
        else if (cmd.compare("LOAD", Qt::CaseInsensitive) == 0) {
            on_btnLoadCode_clicked();
            return;
        }
        else if (cmd.compare("CLEAR", Qt::CaseInsensitive) == 0) {
            on_btnClearCode_clicked();
            return;
        }
        else if (cmd.compare("QUIT", Qt::CaseInsensitive) == 0) {
            QApplication::quit();
            return;
        }
        else if (cmd.compare("HELP", Qt::CaseInsensitive) == 0) {
            ui->textBrowser->append("Help:\n- Type 'LineNumber Code' to edit.\n- Type 'RUN/LOAD/CLEAR/QUIT' to control.\n- Type 'PRINT/LET/INPUT ...' to execute immediately.");
            return;
        }

        // === 情况 C: 立即执行语句 (Immediate Execution) ===
        // 没有行号，也不是命令，尝试当作语句执行
        try {
            Parser parser(cmd.toStdString());
            Statement *stmt = parser.parseStatement();

            // 检查是否允许立即执行
            // 题目要求：LET, PRINT, INPUT 可以立即执行
            // GOTO, IF, REM, END 必须有行号
            if (dynamic_cast<LetStmt*>(stmt) ||
                dynamic_cast<PrintStmt*>(stmt) ||
                dynamic_cast<InputStmt*>(stmt)) {

                // 执行 (使用 globalContext)
                stmt->execute(globalContext);
            }
            else {
                ui->textBrowser->append("Error: This statement requires a line number.");
            }

            delete stmt; // 用完即删
        }
        catch (std::exception &e) {
            // 解析失败，说明不是合法的 Basic 语句，也不是命令
            ui->textBrowser->append("Error: Unknown command or syntax error.");
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

    // 【新增】只有点击 CLEAR 时才清空变量表
    globalContext.clear();
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

//RUN
void MainWindow::on_btnRunCode_clicked()
{
    // 1. 清理 UI
    ui->treeDisplay->clear();
    ui->textBrowser->clear();

    if (programCode.empty()) return;
    //2.不再重置变量表

    // 3. 解析阶段 (Parsing Phase)
    // 将代码文本转换为 Statement 对象，并显示语法树
    std::map<int, Statement*> statementMap;

    try {
        for (auto it = programCode.begin(); it != programCode.end(); ++it) {
            int lineNum = it->first;
            QString codeStr = it->second;

            // 调用 Parser 解析当前行
            Parser parser(codeStr.toStdString());
            Statement *stmt = parser.parseStatement();
            statementMap[lineNum] = stmt;

            // === 语法树显示逻辑 (修复版) ===
            // 目标格式: "100 REM ..." (根节点在行号后面，子节点换行缩进)

            // 获取缩进为0的字符串 (例如 "REM\n    Comment...")
            std::string rawTree = stmt->toString(0);
            QString treeStr = QString::fromStdString(rawTree);

            // 去掉末尾可能多余的换行符
            if (treeStr.endsWith('\n')) treeStr.chop(1);

            // 拼接到 treeDisplay
            ui->treeDisplay->append(QString::number(lineNum) + " " + treeStr);
        }
    }
    catch (std::exception &e) {
        ui->textBrowser->append("Syntax Error: " + QString::fromStdString(e.what()));
        // 解析失败，清理已分配的内存
        for (auto pair : statementMap) delete pair.second;
        return;
    }

    // 4. 执行阶段 (Execution Phase)
    try {
        auto it = statementMap.begin();
        while (it != statementMap.end()) {
            Statement *currentStmt = it->second;

            try {
                // 执行语句
                currentStmt->execute(globalContext);

                // 正常执行下一行
                it++;
            }
            catch (GotoSignal &sig) {
                // 捕获 GOTO 信号，查找目标行
                auto targetIt = statementMap.find(sig.targetLine);
                if (targetIt == statementMap.end()) {
                    throw std::runtime_error("Line number not found: " + std::to_string(sig.targetLine));
                }
                it = targetIt; // 跳转迭代器
            }
        }
    }
    catch (EndSignal &) {
        // 捕获 END 信号，正常退出循环
        // ui->textBrowser->append("--- Program Ended ---");
    }
    catch (std::exception &e) {
        // 捕获运行时错误 (如除以0)
        ui->textBrowser->append("Runtime Error: " + QString::fromStdString(e.what()));
    }

    // 5. 内存清理
    for (auto pair : statementMap) {
        delete pair.second;
    }
}

// 【新增】黑科技：命令行原地输入处理
int MainWindow::handleInputFromCommandLine()
{
    // 1. 准备界面
    ui->textBrowser->append(" ? ");
    ui->cmdLineEdit->setFocus();

    // 2. 暂时断开主逻辑连接 (防止冲突)
    // 使用 0, 0 可以断开该信号的所有连接，更彻底
    ui->cmdLineEdit->disconnect(SIGNAL(editingFinished()));

    // 3. 准备抓取变量
    QString capturedText;
    QEventLoop loop;

    // 4. 建立临时的连接
    // 逻辑：当按下回车时 -> 1.马上保存文本 -> 2.退出循环
    // 注意：这里使用了 C++11 Lambda 表达式，[&] 表示引用捕获外部变量
    auto conn = connect(ui->cmdLineEdit, &QLineEdit::returnPressed, [&](){
        capturedText = ui->cmdLineEdit->text().trimmed(); // 【关键】立刻抓取！
        loop.quit();
    });

    // 5. 阻塞等待
    loop.exec();

    // 6. 清理现场
    // 断开刚才建立的临时 lambda 连接
    disconnect(conn);

    // 清空输入框 (此时文本已经被 capturedText 保存了，可以放心清空)
    ui->cmdLineEdit->clear();

    // 回显
    ui->textBrowser->append(capturedText);

    // 7. 恢复主逻辑连接
    connect(ui->cmdLineEdit, &QLineEdit::editingFinished, this, &MainWindow::on_cmdLineEdit_editingFinished);

    // 8. 转换并返回
    bool ok;
    int val = capturedText.toInt(&ok);

    // 如果想要更友好的调试信息：
    // ui->textBrowser->append("[Debug] Captured: '" + capturedText + "' -> Int: " + QString::number(val));

    if (!ok) return 0;
    return val;
}
