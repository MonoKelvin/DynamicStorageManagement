#include "MainWindow.h"
#include "ui_mainwindow.h"

#include "MemoryManager.h"
#include "AddProcessWidget.h"
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QButtonGroup>

#define HEX_OUTPUT(_Int_) QString("0x%1").arg(_Int_,8,16,QChar('0'))

bool _isWhitespaceAndComments(const QString &str)
{
    auto _s = str.simplified();
    if(str.isEmpty() || str.startsWith("//") || str == "#end") {
        return true;
    }
    return false;
}

MainWindow::MainWindow(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 随机数发生器
    qsrand(uint(QTime(0, 0, 0).secsTo(QTime::currentTime())));

    /* 初始化变量 */
    mBtnSave = ui->btn_save;
    mBtnOpenFile = ui->btn_openFile;
    mBtnFF = ui->btn_FF;
    mBtnNF = ui->btn_NF;
    mBtnBF = ui->btn_BF;
    mBtnWF = ui->btn_WF;
    mBtnAutoCreateProcess = ui->btn_autoCreateProcess;
    mBtnAddProcess = ui->btn_addProcess;
    mBtnEndProcess = ui->btn_endProcess;
    mProcessTable = ui->tw_processTable;
    mMemoryTable = ui->tw_memoryTable;
    mTimer = new QTimer(this);
    mMemoryManager = new MemoryManager;

    QButtonGroup *_bg = new QButtonGroup(this);
    _bg->addButton(ui->btn_startSystem);
    _bg->addButton(ui->btn_endSystem);
    _bg->setExclusive(true);

    /* 初始化控件内容 */
    mProcessTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mMemoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mMemoryTable->verticalHeader()->setHidden(true);
    mProcessTable->verticalHeader()->setHidden(true);
    ui->btn_startSystem->setChecked(false);
    ui->btn_endSystem->setChecked(true);
    ui->btn_FF->setChecked(true);

    uint _sum = mMemoryManager->CreateMemoryPartition(16);
    ui->lb_memorySize->setText(QString("内存总大小：%1").arg(_sum));
    on_btn_endSystem_clicked();

    /* 初始化表内容 */
    freshMemoryTable();
    freshProcessTable();

    /* 信号与槽 */
    connect(mTimer, &QTimer::timeout, [ = ] {
        mMemoryManager->FreshProcessState(qrand() % 2000);
        freshProcessTable();
        freshMemoryTable();
    });

    connect(mBtnAddProcess, &QPushButton::clicked, [=] {
        AddProcessWidget *_newProcessWidget = new AddProcessWidget;
        _newProcessWidget->setWindowTitle("添加进程");
        _newProcessWidget->exec();

        if(!_newProcessWidget->processName.isEmpty() && _newProcessWidget->size > 0) {
            PCB* pcb = new PCB(_newProcessWidget->processName, _newProcessWidget->size);
            mMemoryManager->DynamicPartitionAllocate(pcb);

            freshProcessTable();
            freshMemoryTable();
        }

        delete _newProcessWidget;
    });

    connect(mBtnFF, &QPushButton::clicked, [=]{
        mMemoryManager->setAlgorithm(EAlgorithms::Algorithm_FF);
    });
    connect(mBtnNF, &QPushButton::clicked, [=]{
        mMemoryManager->setAlgorithm(EAlgorithms::Algorithm_NF);
    });
    connect(mBtnBF, &QPushButton::clicked, [=]{
        mMemoryManager->setAlgorithm(EAlgorithms::Algorithm_BF);
        mMemoryManager->AscendingPartition();
    });
    connect(mBtnWF, &QPushButton::clicked, [=]{
        mMemoryManager->setAlgorithm(EAlgorithms::Algorithm_WF);
        mMemoryManager->DescendingPartition();
    });
}

MainWindow::~MainWindow()
{
    delete mMemoryManager;
    delete ui;
}

void MainWindow::freshProcessTable()
{
    mProcessTable->clearContents();

    int _rowCount = mMemoryManager->getPCBs().length();
    mProcessTable->setRowCount(_rowCount);

    for (int _row = 0; _row < _rowCount; _row++)
    {
        auto _p = mMemoryManager->getPCBs()[_row];
        mProcessTable->setItem(_row, 0, new QTableWidgetItem(_p->getProcessName()));
        mProcessTable->setItem(_row, 1, new QTableWidgetItem(QString("%1").arg(_p->getPID())));
        mProcessTable->setItem(_row, 2, new QTableWidgetItem(HEX_OUTPUT(_p->getAddress())));
        mProcessTable->setItem(_row, 3, new QTableWidgetItem(QString("%1").arg(_p->getSize())));
        mProcessTable->setItem(_row, 4, new QTableWidgetItem(QString("%1ms").arg(_p->getWorkTime())));
    }

    ui->lb_processCount->setText(QString("当前进程数量：%1").arg(_rowCount));
}

void MainWindow::freshMemoryTable()
{
    mMemoryTable->clearContents();

    uint _allocated = 0;
    int _rowCount = mMemoryManager->getPartitionList().length();
    mMemoryTable->setRowCount(_rowCount);

    for (int _row = 0; _row < _rowCount; _row++)
    {
        auto _p = mMemoryManager->getPartitionList()[_row];
        mMemoryTable->setItem(_row, 0, new QTableWidgetItem(QString("%1").arg(_row)));
        mMemoryTable->setItem(_row, 1, new QTableWidgetItem(HEX_OUTPUT(_p->address)));
        mMemoryTable->setItem(_row, 2, new QTableWidgetItem(QString("%1").arg(_p->size)));
        mMemoryTable->setItem(_row, 3, new QTableWidgetItem(_p->isOccupied ? "已分配" : "空闲"));
        if(_p->isOccupied) {
            _allocated += uint(_p->size);
        }
    }

    ui->lb_partitionCount->setText(QString("当前分区数量：%1").arg(_rowCount));
    ui->lb_allocated->setText(QString("已分配内存：%1").arg(_allocated));
}

void MainWindow::on_btn_endProcess_clicked()
{
    mMemoryManager->ClearProcesses();
    freshProcessTable();
    freshMemoryTable();
}

void MainWindow::on_btn_autoCreateProcess_clicked()
{
    static uint sProcessNumber = 1;
    PCB *_pcb = new PCB("process" + QString("%1").arg(sProcessNumber++), qrand() % 6000 + 64);
    mMemoryManager->DynamicPartitionAllocate(_pcb);
    freshProcessTable();
    freshMemoryTable();
}

void MainWindow::on_btn_startSystem_clicked()
{
    mTimer->start(1000);
    mBtnFF->setEnabled(true);
    mBtnNF->setEnabled(true);
    mBtnBF->setEnabled(true);
    mBtnWF->setEnabled(true);
    mBtnAutoCreateProcess->setEnabled(true);
    mBtnAddProcess->setEnabled(true);
    mBtnEndProcess->setEnabled(true);
}

void MainWindow::on_btn_endSystem_clicked()
{
    mTimer->stop();
    mBtnFF->setEnabled(false);
    mBtnNF->setEnabled(false);
    mBtnBF->setEnabled(false);
    mBtnWF->setEnabled(false);
    mBtnAutoCreateProcess->setEnabled(false);
    mBtnAddProcess->setEnabled(false);
    mBtnEndProcess->setEnabled(false);
}

void MainWindow::on_btn_resetMemory_clicked()
{
    if(mMemoryManager->getPCBs().isEmpty()) {
        uint _sum = mMemoryManager->CreateMemoryPartition(16);
        ui->lb_memorySize->setText(QString("内存总大小：%1").arg(_sum));
        freshMemoryTable();
    } else {
        QMessageBox::warning(nullptr, "内存重置出错", "内存中还有部分进程申请块未完成需求，不可重置内存！");
    }
}

void MainWindow::on_btn_openFile_clicked()
{
    // 打开文件前先询问是否要清空当前模拟环境
    if(!mMemoryManager->getPartitionList().isEmpty() || mTimer->isActive()) {
        auto _require = QMessageBox::information(nullptr, "询问",
                                                 "当前正在运行一个模拟环境，是否清空当前工作环境？",
                                                 QMessageBox::Yes | QMessageBox::No);
        if(_require == QMessageBox::Yes) {
            mMemoryManager->ClearProcesses();
            mMemoryManager->ClearMemory();
            freshProcessTable();
            freshMemoryTable();
            ui->lb_memorySize->setText("内存总大小：0");
        } else {
            return;
        }
    }

    QFile _file(QFileDialog::getOpenFileName(this, "打开文件", QDir().currentPath(),
                                             "内存分区模拟文件(*.dsm);;文本文件(*.txt);"));
    if(!_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream _inputStream(&_file);
    QString _line = "";
    while(!_inputStream.atEnd())
    {
        _line = _inputStream.readLine();

        if(_line == "#end") {
            break;
        }

        if(_line == "#config") {
            do {
                _line = _inputStream.readLine();
                if (_isWhitespaceAndComments(_line)) {
                    continue;
                }

                if (_line.startsWith("algorithm")) {
                    auto _algorithm = _line.right(2);
                    if(_algorithm == "FF") {
                        mMemoryManager->setAlgorithm(EAlgorithms::Algorithm_FF);
                    } else if(_algorithm == "NF") {
                        mMemoryManager->setAlgorithm(EAlgorithms::Algorithm_NF);
                    } else if(_algorithm == "BF") {
                        mMemoryManager->setAlgorithm(EAlgorithms::Algorithm_BF);
                    } else if(_algorithm == "WF") {
                        mMemoryManager->setAlgorithm(EAlgorithms::Algorithm_WF);
                    } else {
                        QMessageBox::critical(nullptr, "解析文件失败", "无法识别算法类型！");
                    }
                }

                if(_line.startsWith("isCompact")) {
                    if (_line.endsWith("true")) {
                        on_btn_compact_toggled(true);
                    } else {
                        on_btn_compact_toggled(false);
                    }
                }
            } while (!_line.startsWith("#"));
        }

        if(_line == "#partitions") {
            int _addr = 0;
            int _size = 0;

            do {
                _line = _inputStream.readLine();
                if (_isWhitespaceAndComments(_line)) {
                    continue;
                }

                QStringList _parStr = _line.split("\t");
                _size = _parStr[0].toInt();
                auto _p = new Partition(_addr, _size, false);
                if(_parStr.length() > 1) {
                    _p->isOccupied = true;
                    PCB *_pcb = new PCB(_parStr[1], _size);
                    _pcb->setPartition(_p);
                    _pcb->setWorkTime(_parStr[2].toInt());
                    mMemoryManager->getPCBs().push_back(_pcb);
                }

                mMemoryManager->getPartitionList().push_back(_p);
                _addr += (_size + 1);
            } while(!_line.startsWith("#"));

            ui->lb_memorySize->setText(QString("内存总大小：%1").arg(_addr));
        }
    }

    _file.close();
    freshProcessTable();
    freshMemoryTable();
}

void MainWindow::on_btn_save_clicked()
{
    // 保存前先暂停模拟
    on_btn_endSystem_clicked();

    QFile _file(QFileDialog::getSaveFileName(this, "保存文件", QDir().currentPath(),
                                             "内存分区模拟文件(*.dsm);;文本文件(*.txt);"));

    if(!_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream _outputStream(&_file);

    /** 保存配置信息 */
    _outputStream << "#confing" << endl;
    switch (mMemoryManager->getAlgorithm()) {
    case EAlgorithms::Algorithm_FF:
        _outputStream << "algorithm = FF" << endl;
        break;
    case EAlgorithms::Algorithm_NF:
        _outputStream << "algorithm = NF" << endl;
        break;
    case EAlgorithms::Algorithm_BF:
        _outputStream << "algorithm = BF" << endl;
        break;
    case EAlgorithms::Algorithm_WF:
        _outputStream << "algorithm = WF" << endl;
        break;
    case EAlgorithms::Unknown:
        _outputStream << "algorithm = None" << endl;
        break;
    }

    {
        QString _isCompact = mMemoryManager->getIsCompact() ? "true" : "false";
        _outputStream << "isCompact = " << _isCompact << endl;
    }

    /** 保存分区分配信息 */
    _outputStream << "\n#partitions" << endl;
    _outputStream << "//分区大小 进程名 剩余工作时间" << endl;
    for (auto iter = mMemoryManager->getPartitionList().begin();
         iter != mMemoryManager->getPartitionList().end(); iter++) {
        _outputStream << (*iter)->size;
        if((*iter)->isOccupied) {
            for (auto ipcb = mMemoryManager->getPCBs().begin();
                 ipcb != mMemoryManager->getPCBs().end(); ipcb++) {
                if((*ipcb)->getPartition() == *iter) {
                    _outputStream << "\t" << (*ipcb)->getProcessName() << "\t"
                                  << (*ipcb)->getWorkTime();
                    break;
                }

                if(ipcb == mMemoryManager->getPCBs().end()) {
                    QMessageBox::critical(nullptr, "文件保存出错", "无法获取进程信息，请尝试重新分配分区！");
                    _file.close();
                    _file.open(QFile::WriteOnly | QFile::Truncate);
                    _file.close();
                    return;
                }
            }
        }
        _outputStream << endl;
    }

    _outputStream << "#end";
    _file.close();
}

void MainWindow::on_btn_compact_toggled(bool checked)
{
    mMemoryManager->setIsCompact(checked);
}
