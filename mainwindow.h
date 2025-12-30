#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>  // 【新增】用于存储代码
#include "expression.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_cmdLineEdit_editingFinished(); // 处理回车
    void on_btnLoadCode_clicked();         // 【新增】处理 LOAD 按钮
    void on_btnClearCode_clicked();        // 【新增】处理 CLEAR 按钮
    void on_btnRunCode_clicked();

private:
    Ui::MainWindow *ui;

    // 【新增】核心数据结构：存储 BASIC 程序代码
    // Key (int): 行号
    // Value (QString): 代码内容
    std::map<int, QString> programCode;

    // 【新增】全局上下文，用于存储变量
    // 这样我们在立即模式下定义的变量 (LET A=10) 才能被后面的 PRINT A 访问
    EvaluationContext globalContext;

    // 【新增】辅助函数：将 map 中的代码刷新显示到 CodeDisplay
    void refreshCodeDisplay();
};
#endif // MAINWINDOW_H
