#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "DataStructure.h"

// 动态内存管理类
class MemoryManager
{
public:
    MemoryManager();
    ~MemoryManager();

    /**
     * @brief 动态分区分配
     * @param pcb 要进行分配的PCB
     */
    void DynamicPartitionAllocate(PCB *pcb);

    /**
     * @brief 创建内存分区
     * @param count 生成内存分区个数
     * @param minSize 每个最分区最小值
     * @param maxSize 每个最分区最大值
     * @param isRandom 是否随机生成，
     * @return uint 返回创建的内存总大小
     * @note 当 isRandom == false 时，默认分区大小全为 maxSize
     */
    uint CreateMemoryPartition(int count,
                               int minSize = 256,
                               int maxSize = 8192,
                               bool isRandom = true);

    // 内存回收
    void Free(PCB* pcb);

    // 清空所有进程
    void ClearProcesses();

    // 清空所有内存分区，包括已分配的进程申请部分也会一并清空
    void ClearMemory();

    // 按照分区的大小升序排序内存
    void AscendingPartition();

    // 按照分区的大小升序排序内存
    void DescendingPartition();

    // 更新进程状态
    void FreshProcessState(int delta);

    inline QList<Partition*>& getPartitionList()
    {
        return mPartitionList;
    }

    inline QList<PCB*>& getPCBs()
    {
        return mPCBs;
    }

    inline void setAlgorithm(EAlgorithms algorithms)
    {
        eAlgorithm = algorithms;
    }

    inline EAlgorithms getAlgorithm() const
    {
        return eAlgorithm;
    }

    inline void setIsCompact(bool isCompact)
    {
        mIsCompact = isCompact;
    }

    inline bool getIsCompact() const
    {
        return mIsCompact;
    }

private:

    // 首次适应算法
    void FirstFitAlgorithm(PCB* pcb);

    // 循环首次适应算法，第二个参数是下一个要开始遍历的分区号
    int NextFitAlgorithm(PCB* pcb, int startNum);

    // 最佳适应算法
    void BestFitAlgorithm(PCB* pcb);

    // 最坏适应算法
    void WorstFitAlgorithm(PCB* pcb);

    // 紧凑算法
    void CompactAlgotithm();

    // 动态分区分配算法
    EAlgorithms eAlgorithm;

    // 是否支持紧凑算法
    bool mIsCompact;

    // 分区链表
    QList<Partition*> mPartitionList;

    // 未结束的进程控制块链表
    QList<PCB*> mPCBs;
};

#endif // MEMORYMANAGER_H
