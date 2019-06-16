#ifndef DSMWINDOW_H
#define DSMWINDOW_H

#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include "DataStructure.h"

class MemoryManager;

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_endProcess_clicked();
    void on_btn_autoCreateProcess_clicked();
    void on_btn_startSystem_clicked();
    void on_btn_endSystem_clicked();

    void on_btn_resetMemory_clicked();

    void on_btn_openFile_clicked();

    void on_btn_save_clicked();

    void on_btn_compact_toggled(bool checked);

private:

    // 更新进程状态表
    void freshProcessTable();

    // 更新内存分区表
    void freshMemoryTable();

private:
    Ui::MainWindow* ui;

    /* 计时器，1s为一个周期
     */
    QTimer* mTimer;

    QPushButton* mBtnSave;
    QPushButton* mBtnOpenFile;

    QPushButton* mBtnFF;
    QPushButton* mBtnNF;
    QPushButton* mBtnBF;
    QPushButton* mBtnWF;

    QPushButton* mBtnAutoCreateProcess;
    QPushButton* mBtnAddProcess;
    QPushButton* mBtnEndProcess;

    QTableWidget* mProcessTable;        // 进程执行情况表
    QTableWidget* mMemoryTable;         // 内存分区分配状况表
    MemoryManager* mMemoryManager;      // 内存管理类
};

#endif // DSMWINDOW_H
