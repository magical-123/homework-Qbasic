#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog> // 用于打开文件
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
