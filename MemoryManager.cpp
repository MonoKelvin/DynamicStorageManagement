#include "MemoryManager.h"

#include <QDebug>
#include <QMessageBox>

QList<int> PCB::PIDs = QList<int>();

inline int _generateRandomWorkTime()
{
    //生成随机工作时间为1~30s(1000ms ~ 30000ms)
    return (qrand() % 30000) + 1000;
}

MemoryManager::MemoryManager()
    : eAlgorithm(EAlgorithms::Algorithm_FF)
    , mIsCompact(false)
{

}

MemoryManager::~MemoryManager()
{
    ClearMemory();
}

void MemoryManager::DynamicPartitionAllocate(PCB *pcb)
{
    static int sCurPartitionNum = 0;

    if(mIsCompact) {
        CompactAlgotithm();
    }

    switch (eAlgorithm) {
    case EAlgorithms::Algorithm_FF:
        FirstFitAlgorithm(pcb);
        break;
    case EAlgorithms::Algorithm_NF:
    {
        int _temp = sCurPartitionNum;
        sCurPartitionNum = NextFitAlgorithm(pcb, sCurPartitionNum);
        if(sCurPartitionNum == -1) {
            sCurPartitionNum = _temp;
        }
    }
        break;
    case EAlgorithms::Algorithm_BF:
        BestFitAlgorithm(pcb);
        break;
    case EAlgorithms::Algorithm_WF:
        WorstFitAlgorithm(pcb);
        break;
    case EAlgorithms::Unknown:
        QMessageBox::warning(nullptr, "无法分配内存", "请在左侧指定分配内存所用的算法！");
        break;
    }

    if(mIsCompact) {
        std::sort(mPartitionList.begin(), mPartitionList.end(), [=](Partition *a, Partition *b){
            return a->address < b->address;
        });
    }
}

uint MemoryManager::CreateMemoryPartition(int count, int minSize, int maxSize, bool isRandom)
{
    int _addr = 0;
    uint _sum = 0;

    ClearMemory();

    if (isRandom) {
        for (int i = 0; i < count; i++) {
            // 新建一个分区
            int _size = qrand() % (maxSize - minSize) + minSize;
            mPartitionList.append(new Partition(_addr, _size));
            _sum += uint(_size);

            // 生成随机分区大小
            _addr += _size;
        }
    } else {
        for (int i = 0; i < count; i++) {
            mPartitionList.append(new Partition(_addr, maxSize));
            _addr += maxSize;
        }
        _sum = uint(count * maxSize);
    }

    return _sum;
}

void MemoryManager::FirstFitAlgorithm(PCB* pcb)
{
    // 遍历内存寻找空闲分区
    auto iter = mPartitionList.begin();
    for (; iter != mPartitionList.end(); iter++) {
        auto _p = (*iter);

        // 当前分区未被占用 && 当前分区的大小 >= 进程申请的大小
        if (!_p->isOccupied && _p->size >= pcb->mSize) {

            // 当前分区状态修改为被占用
            _p->isOccupied = true;

            // 更改进程首地址为分区的首地址
            pcb->mPartition = _p;

            // 把进程存入 mPCBs 进程容器中，并为其随机生成作业耗时
            pcb->mWorkTime = _generateRandomWorkTime();

            mPCBs.push_back(pcb);

            // 如果内存分区大小刚好等于申请的大小就不要再分裂新的分区
            if (_p->size != pcb->mSize) {
                // 分裂出一个新的分区：
                // 地址为 = 当前分区地址 + 当前分区大小
                // 剩余大小 = 当前分区大小 - 进程申请的大小
                // 修改为未被占用
                Partition* _newp = new Partition(_p->address + pcb->mSize,
                                                 _p->size - pcb->mSize);

                // 当前分区大小变为进程申请的大小
                _p->size = pcb->mSize;

                // 插入到当前分区的后面
                mPartitionList.insert((iter + 1), _newp);
            }

            // 分配完成就退出
            break;
        }
    }

    // 未到符合条件的分区，则分配失败
    if (iter == mPartitionList.end()) {
        QMessageBox::warning(nullptr, "内存分配异常",
                             QString("没有满足进程需求的最大分区！"
                                     "系统已拒绝该进程请求\n当前进程请求量：%1")
                                 .arg(pcb->getSize()));
        SAFE_DELETE(pcb);
    }
}

int MemoryManager::NextFitAlgorithm(PCB* pcb, int startNum)
{
    int _num = startNum;
    int _len = mPartitionList.length();
    bool _isRestart = false;
    auto _p = mPartitionList[_num];

    // 遍历内存寻找空闲分区
    for (; _num < _len; _num++) {
        // 当到末尾都没有找到合适的分区，那么就从头开始
        if (_num >= _len - 1) {
            _num = 0;
            _isRestart = true;
        }

        // 当前分区未被占用 && 当前分区的大小 >= 进程申请的大小
        if (_p->isOccupied == false && _p->size >= pcb->mSize) {
            // 当前分区状态修改为被占用
            _p->isOccupied = true;

            // 更改进程首地址为分区的首地址
            pcb->mPartition = _p;

            // 把进程存入 mPCBs 进程容器中
            pcb->mWorkTime = _generateRandomWorkTime();
            mPCBs.push_back(pcb);

            if (_p->size != pcb->mSize) {
                // 分裂出一个新的分区：
                // 地址为 = 当前分区地址 + 当前分区大小 + 1
                // 剩余大小 = 当前分区大小 - 进程申请的大小
                // 修改为未被占用
                Partition* _newp = new Partition(_p->address + pcb->mSize, _p->size - pcb->mSize);

                // 当前分区大小变为进程申请的大小
                _p->size = pcb->mSize;

                // 插入到当前分区的后面
                mPartitionList.insert(_num + 1, _newp);
            }

            return _num;
        }

        // 若从头找还是没找到合适的，则分配失败
        if (_isRestart && _num >= startNum) {
            SAFE_DELETE(pcb);
            QMessageBox::information(nullptr,
                                     "内存分配异常",
                                     "没有满足进程需求的最大分区大小！系统已拒绝该进程请求");
            return -1;
        }

        _p = mPartitionList[_num];
    }
    return -1;
}

void MemoryManager::BestFitAlgorithm(PCB* pcb)
{
//    AscendingPartition();
    FirstFitAlgorithm(pcb);
    AscendingPartition();
}

void MemoryManager::WorstFitAlgorithm(PCB* pcb)
{
//    DescendingPartition();
    FirstFitAlgorithm(pcb);
    DescendingPartition();
}

void MemoryManager::CompactAlgotithm()
{
    /**
     * 1.先把已分配的内存分区重定位其地址，使之紧凑连续
     * 2.再把剩余的小分区排列在后方形成一个大的分区块
     */
    int _addr = 0;
    Partition *_p = new Partition(0, 0);
    for (auto iter = mPartitionList.begin(); iter != mPartitionList.end(); iter++) {
        if ((*iter)->isOccupied) {
            // 地址重定位
            (*iter)->address = _addr;
            _addr += (*iter)->size;

            // 合并那些小的分区
            auto _nextp = (*(iter + 1));
            if(_nextp != nullptr && !_nextp->isOccupied) {
                _p->size += _nextp->size;

                iter = mPartitionList.erase(iter + 1);
                SAFE_DELETE(_nextp);
                if(iter == mPartitionList.end()) {
                    break;
                }
            }
        }
    }

    _p->address = _addr;
    mPartitionList.push_back(_p);

    for (auto iter = mPartitionList.begin(); iter != mPartitionList.end(); iter++) {
        if(!(*iter)->isOccupied) {
            // 地址重定位
            (*iter)->address = _addr;
            _addr += (*iter)->size;
        }
    }
}

void MemoryManager::Free(PCB* pcb)
{
    // 在mPCBs中查找是否存在该进程
    if (mPCBs.contains(pcb)) {
        // 在分区链表中找到被该进程的所占用的块
        auto iter = mPartitionList.begin();
        for (; iter != mPartitionList.end(); iter++) {
            if(*iter == pcb->mPartition) {
                break;
            }
        }

        // 获得该内存分区块
        Partition* _p = *iter;

        // 该内存分区的状态更改为未占用
        _p->isOccupied = false;

        /**
         * 1.如果所回收的内存分区[后面]存在空闲的内存分区
         * 尾部后面一定没有空闲分区
         */
        if (_p != mPartitionList.last()) {
            Partition* _nextp = *(iter + 1);
            if (_nextp->isOccupied == false) {
                // 把当前分区的大小 += 后一个内存大小
                _p->size += _nextp->size;

                // 取消占用
                _p->isOccupied = false;

                // 从分区表中移除
                mPartitionList.removeOne(_nextp);

                // 释放内存空间
                SAFE_DELETE(_nextp);
            }
        }

        /** 2.如果所回收的内存分区[前面]存在空闲的内存分区
         * 但头部前面一定没有空闲分区
         */
        if (_p != mPartitionList.first()) {
            Partition* _prep = *(iter - 1);
            if (_prep->isOccupied == false) {
                // 把前面分区的大小 += 当前内存大小
                _prep->size += _p->size;

//                // 重新指定内存分区
//                pcb->mPartition = _prep;

                // 从分区表中移除当前占用分区
                mPartitionList.removeOne(_p);

                // 释放内存空间
                SAFE_DELETE(_p);
            }
        }

        /** 3.释放进程：从进程链表中删除该进程
         */
        mPCBs.removeOne(pcb);
        SAFE_DELETE(pcb);

        /** 4.根据算法类型决定是否进行排序分区
         */
        if (eAlgorithm == EAlgorithms::Algorithm_BF) {
            AscendingPartition();
        } else if (eAlgorithm == EAlgorithms::Algorithm_WF) {
            DescendingPartition();
        }

    } else {
        QMessageBox::warning(nullptr, "无效进程", "内存中不存在该进程申请块！");
        SAFE_DELETE(pcb);
    }
}

void MemoryManager::ClearProcesses()
{
    for (auto iter = mPCBs.begin(); iter != mPCBs.end(); iter++) {
        Free(*iter);
    }
}

void MemoryManager::ClearMemory()
{
    // 释放进程控制块
    qDeleteAll(mPCBs);
    mPCBs.clear();

    // 释放分区链表
    qDeleteAll(mPartitionList);
    mPartitionList.clear();
}

void MemoryManager::AscendingPartition()
{
    // 分区大小升序排序
    std::sort(mPartitionList.begin(), mPartitionList.end(), [ = ](Partition * a, Partition * b)
    {
        return a->size < b->size;
    });

    // 重新计算分区首地址
    int _addr = 0;
    for (auto iter = mPartitionList.begin(); iter != mPartitionList.end(); iter++)
    {
        (*iter)->address = _addr;
        _addr = _addr + (*iter)->size;
    }
}

void MemoryManager::DescendingPartition()
{
    // 分区大小升序排序
    std::sort(mPartitionList.begin(), mPartitionList.end(), [ = ](Partition * a, Partition * b)
    {
        return a->size > b->size;
    });

    // 重新计算分区首地址
    int _addr = 0;
    for (auto iter = mPartitionList.begin(); iter != mPartitionList.end(); iter++)
    {
        (*iter)->address = _addr;
        _addr = _addr + (*iter)->size;
    }
}

void MemoryManager::FreshProcessState(int delta)
{
    for (auto iter = mPCBs.begin(); iter != mPCBs.end(); iter++) {
        // 每次更新时作业时长就减少delta
        (*iter)->mWorkTime -= delta;

        // 当作业结束时，释放该进程
        if ((*iter)->mWorkTime <= 0) {
            Free(*iter);
            break;
        }
    }
}
