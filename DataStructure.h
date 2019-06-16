#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <QList>
#include <QString>
#include <time.h>

typedef unsigned int uint;

#define SAFE_DELETE(_Pointer_) do{if(!_Pointer_){delete _Pointer_; _Pointer_ = nullptr;}}while(0)

// 分区结构
struct Partition {
    int address;       // 分区首地址
    int size;          // 分区的块大小
    bool isOccupied;    // 该分区是否被占用

    Partition() {}

    Partition(int _address, int _size, bool occupied = false)
        : address(_address)
        , size(_size)
        , isOccupied(occupied)
    {}
};

// 进程控制块
class PCB
{
    friend class MemoryManager;
public:
    PCB() {}

    /** 构造函数，会自动分配进程ID号
     */
    PCB(const QString name, int size = 1024)
        : mName(name)
        , mSize(size)
    {
        mPID = generateDifferentPID();
    }

    inline QString getProcessName() const {return mName;}
    inline int getSize() const {return mSize;}
    inline int getPID() const {return mPID;}
    inline int getAddress() const {return mPartition->address;}
    inline int getWorkTime() const {return mWorkTime;}

    inline void setWorkTime(int time) {mWorkTime = time;}

    /** 分配分区
     */
    inline void setPartition(Partition *par)
    {
        if(par->size >= mSize) {
            mPartition = par;
        } else {
            mPartition = nullptr;
        }
    }

    inline Partition* getPartition() const
    {
        return mPartition;
    }

private:
    QString mName;              // 进程名
    int mSize;                  // 进程申请内存的大小
    Partition *mPartition;      // 进程申请的内存的分区地址
    int mPID;                   // 进程 ID
    int mWorkTime;              // 用来模拟工作时间的参数，单位是ms

private:
    //保存所有进程的 ID 表
    static QList<int> PIDs;

    // 生成一个不同的 PID
    inline int generateDifferentPID()
    {
        int _id = 0;
        do {
            _id = rand() % 65536;
        } while (PIDs.contains(_id));

        PIDs.push_back(_id);
        return _id;
    }
};


// 算法类型
enum class EAlgorithms {
    Algorithm_FF,       // first fit
    Algorithm_NF,       // next fit
    Algorithm_BF,       // best fit
    Algorithm_WF,       // worst fit
    Unknown,            // 未定义
};


#endif // DATASTRUCTURE_H
