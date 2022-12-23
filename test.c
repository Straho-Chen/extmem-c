#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

Buffer buf; /* A buffer */
unsigned int write_blk_addr;

void getBlock(unsigned char **blk)
{
    *blk = getNewBlockInBuffer(&buf);
}

void readBlock(unsigned char **blk, unsigned int addr, int is_print)
{
    /* Read the block from the hard disk */
    if ((*blk = readBlockFromDisk(addr, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return;
    }
    if (is_print)
        printf("读入数据块%d\n", addr);
}

void getXY(int *X, int *Y, unsigned char *blk, unsigned int addr)
{
    char str[5];
    for (int k = 0; k < 4; k++)
    {
        str[k] = *(blk + addr * 8 + k);
    }
    *X = atoi(str);
    for (int k = 0; k < 4; k++)
    {
        str[k] = *(blk + addr * 8 + 4 + k);
    }
    *Y = atoi(str);
}

void fillWithXY(int X, int Y, unsigned char *blk, unsigned int addr)
{
    char str[5];
    sprintf(str, "%d", X);
    for (int i = 0; i < 4; i++)
    {
        *(blk + i + addr * 8) = str[i];
    }
    if (Y == 0)
        return;
    sprintf(str, "%d", Y);
    for (int i = 0; i < 4; i++)
    {
        *(blk + i + addr * 8 + 4) = str[i];
    }
}

void writeBlock(unsigned char *blk)
{
    /* Write the block to the hard disk */
    fillWithXY(write_blk_addr + 1, 0, blk, 7);
    if (writeBlockToDisk(blk, write_blk_addr, &buf) != 0)
    {
        perror("Writing Block Failed!\n");
        return;
    }
    printf("注：结果写入磁盘：%d\n", write_blk_addr);
    write_blk_addr++;
}

void freeBlock(unsigned char *blk)
{
    freeBlockInBuffer(blk, &buf);
}

void fillOutputBlockWith1Item(int *addr, int X, int Y, unsigned char **blk)
{
    if (*addr < 6)
    {
        if (*addr == -1)
        {
            getBlock(blk);
            *addr = 0;
        }
        fillWithXY(X, Y, *blk, *addr);
        *addr += 1;
    }
    else
    {
        fillWithXY(X, Y, *blk, *addr);
        writeBlock(*blk);
        *addr = -1;
    }
}

/**
 * task1
 */
void linearSelect()
{
    printf("-----------------------------\n");
    printf("基于线性搜索的选择算法 S.C=128:\n");
    printf("-----------------------------\n");
    unsigned char *blk;
    unsigned char *write_blk;
    int X = -1;
    int Y = -1;
    int addr = -1;
    buf.numIO = 0;
    int count = 0;
    for (int i = 17; i <= 48; i++)
    {
        readBlock(&blk, i, 1);
        for (int j = 0; j < 7; j++)
        {
            getXY(&X, &Y, blk, j);
            if (X == 128)
            {
                printf("(%d, %d)\n", X, Y);
                fillOutputBlockWith1Item(&addr, X, Y, &write_blk);
                count++;
            }
        }
        freeBlock(blk);
    }
    writeBlock(write_blk);
    printf("满足选择条件的元组一共%d个。\n", count);
    printf("IO读写一共%ld次。\n", buf.numIO);
}

/**
 * task2
 */
void innerSort(unsigned char **blks, unsigned int len)
{
    int X1 = -1;
    int Y1 = -1;
    int X2 = -1;
    int Y2 = -1;
    for (int i = 0; i < len - 1; i++)
    {
        for (int k = 0; k < len - 1 - i; k++)
        {
            getXY(&X1, &Y1, blks[k / 7], k % 7);
            getXY(&X2, &Y2, blks[(k + 1) / 7], (k + 1) % 7);
            if (X1 > X2)
            {
                fillWithXY(X1, Y1, blks[(k + 1) / 7], (k + 1) % 7);
                fillWithXY(X2, Y2, blks[k / 7], k % 7);
            }
        }
    }
}

unsigned int getMin(int *min_X, unsigned char *blk, unsigned int len)
{
    int X = -1;
    int Y = -1;
    unsigned int index = 0;
    *min_X = 9999;
    for (int i = 0; i < len; i++)
    {
        getXY(&X, &Y, blk, i);
        if (X == 9999)
            continue;
        if (X < *min_X)
        {
            index = i;
            *min_X = X;
        }
    }
    return index;
}

/*
 *输入当前的XY，输出下一个XY
 */
int getNextXY(int *X, int *Y, unsigned char *blk)
{
    char str[5];
    int give_X = *X;
    int give_Y = *Y;
    int find_X = -1;
    int find_Y = -1;
    for (int i = 0; i < 6; i++)
    {
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(blk + i * 8 + k);
        }
        find_X = atoi(str);
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(blk + i * 8 + 4 + k);
        }
        find_Y = atoi(str);
        if (find_X == give_X && find_Y == give_Y)
        {
            for (int k = 0; k < 4; k++)
            {
                str[k] = *(blk + (i + 1) * 8 + k);
            }
            *X = atoi(str);
            for (int k = 0; k < 4; k++)
            {
                str[k] = *(blk + (i + 1) * 8 + 4 + k);
            }
            *Y = atoi(str);
            return 1;
        }
    }
    return 0;
}

int getNextBlock(int *X, int *Y, unsigned char **blk, unsigned int max_addr)
{
    int next_block = -1;
    getXY(X, Y, *blk, 7);
    next_block = *X;
    if (next_block == max_addr)
        return 0;
    freeBlock(*blk);
    readBlock(blk, next_block, 0);
    getXY(X, Y, *blk, 0);
    return 1;
}

void TPMMS(unsigned int addr_start, unsigned int addr_end)
{
    printf("--------------------------\n");
    if (addr_start == 1)
        printf("利用TPMMS算法对关系R进行排序\n");
    else
        printf("利用TPMMS算法对关系S进行排序\n");
    printf("--------------------------\n");
    unsigned int wirte_addr_begin = write_blk_addr;
    unsigned char *blks[buf.numAllBlk];
    int totalBlocks = addr_end - addr_start + 1;
    int groupNum = (totalBlocks % buf.numAllBlk == 0) ? (totalBlocks / buf.numAllBlk) : (totalBlocks / buf.numAllBlk + 1);
    int X = -1;
    int Y = -1;
    for (int i = 0; i < groupNum; i++)
    {
        for (int k = 0; k < buf.numAllBlk; k++)
        {
            readBlock(&blks[k], addr_start + i * buf.numAllBlk + k, 0);
        }
        innerSort(blks, 7 * buf.numAllBlk);
        for (int k = 0; k < buf.numAllBlk; k++)
        {
            writeBlock(blks[k]);
        }
    }
    int compare = groupNum;
    getBlock(&blks[compare]);
    int output = compare + 1;
    for (int i = 0; i < groupNum; i++)
    {
        readBlock(&blks[i], write_blk_addr - totalBlocks + i * buf.numAllBlk, 0); // read first block in each group
        getXY(&X, &Y, blks[i], 0);
        fillWithXY(X, Y, blks[compare], i);
    }
    write_blk_addr = wirte_addr_begin + 100; // wirte address for finally merged output
    int p = -1;
    int finished = 0;
    for (;;)
    {
        if (finished == groupNum)
            break;
        unsigned int min_addr = getMin(&X, blks[compare], groupNum);
        if (min_addr != 0 || X != 0)
        {
            getXY(&X, &Y, blks[compare], min_addr);
            fillOutputBlockWith1Item(&p, X, Y, &blks[output]);
            if (getNextXY(&X, &Y, blks[min_addr]))
            {
                fillWithXY(X, Y, blks[compare], min_addr); // goto getMin
                continue;
            }
            else
            {
                if (getNextBlock(&X, &Y, &blks[min_addr], (min_addr + 1) * buf.numAllBlk + wirte_addr_begin))
                {
                    fillWithXY(X, Y, blks[compare], min_addr); // goto getMin
                    continue;
                }
                else
                {
                    fillWithXY(9999, 9999, blks[compare], min_addr);
                    finished++;
                    continue;
                }
            }
        }
    }
    freeBlock(blks[compare]);
    for (int i = 0; i < groupNum; i++)
        freeBlock(blks[i]);
}

/**
 * task 3
 */
void initIndex(unsigned int groupNum, unsigned int blksPerGroup, unsigned int startAddr, unsigned int outputAddr)
{
    unsigned char *blks[groupNum + 1];
    int output = groupNum;
    int X = -1;
    int Y = -1;
    int addr = -1;
    write_blk_addr = outputAddr; // index output address
    for (int i = 0; i < groupNum; i++)
    {
        readBlock(&blks[i], startAddr + i * blksPerGroup, 1); // read first block in each group
        getXY(&X, &Y, blks[i], 0);
        fillOutputBlockWith1Item(&addr, X, startAddr + i * blksPerGroup, &blks[output]);
        freeBlock(blks[i]);
    }
    writeBlock(blks[output]);
}

void indexBasedSelect(unsigned int indexAddr, unsigned int blksPerGroup, unsigned int indexBlksNum, unsigned int resultAddr)
{
    printf("-----------------------------\n");
    printf("基于索引的关系选择算法 S.C=128:\n");
    printf("-----------------------------\n");
    unsigned char *blks[buf.numAllBlk];
    int index = 0;
    readBlock(&blks[index], indexAddr, 1);
    int X = -1;
    int Y = -1;
    int startGroupNum = -1;
    int endGroupNum = -1;
    int i = 0;
    getXY(&X, &Y, blks[index], i);
    for (;;)
    {
        if (X < 128)
        {
            startGroupNum = Y;
            endGroupNum = Y;
        }
        else if (X > 128)
        {
            endGroupNum = Y;
            break;
        }
        if (!getNextXY(&X, &Y, blks[index]))
            if (!getNextBlock(&X, &Y, &blks[index], indexAddr + indexBlksNum))
                break;
        i++;
    }
    freeBlock(blks[index]);
    int count = 0;
    int addr = -1;
    int finish = 0;
    write_blk_addr = resultAddr;
    for (int i = startGroupNum; i < endGroupNum; i += blksPerGroup)
    {
        for (int j = 0; j < blksPerGroup; j++)
        {
            readBlock(&blks[0], i + j, 1);
            for (int k = 0; k < 7; k++)
            {
                getXY(&X, &Y, blks[0], k);
                if (X == 128)
                {
                    printf("(%d, %d)\n", X, Y);
                    fillOutputBlockWith1Item(&addr, X, Y, &blks[1]);
                    count++;
                }
                else if (X > 128)
                {
                    finish = 1;
                    break;
                }
            }
            freeBlock(blks[0]);
            if (finish)
                break;
        }
        if (finish)
            break;
    }
    writeBlock(blks[1]);
    printf("满足选择条件的元组一共%d个。\n", count);
    printf("IO读写一共%ld次。\n", buf.numIO);
}

/**
 * task4
 */
void fillOutputBlockWith2Item(int *addr, int SX, int SY, int RX, int RY, unsigned char **blk)
{
    if (*addr == -1)
    {
        getBlock(blk);
        *addr = 0;
    }
    if (*addr % 2 == 0)
    {
        if (*addr <= 4)
        {
            fillWithXY(SX, SY, *blk, *addr);     // fill S
            fillWithXY(RX, RY, *blk, *addr + 1); // fill R
            *addr += 2;
        }
        else if (*addr == 6)
        {
            fillWithXY(SX, SY, *blk, *addr); // fill S
            writeBlock(*blk);
            getBlock(blk);
            *addr = 0;
            fillWithXY(RX, RY, *blk, *addr); // fill R
            *addr += 1;
        }
    }
    else
    {
        if (*addr < 5)
        {
            fillWithXY(SX, SY, *blk, *addr);
            fillWithXY(RX, RY, *blk, *addr + 1);
            *addr += 2;
        }
        else
        {
            fillWithXY(SX, SY, *blk, *addr);
            fillWithXY(RX, RY, *blk, *addr + 1);
            writeBlock(*blk);
            *addr = -1;
        }
    }
}

void rollBack(unsigned int p, unsigned char **blk, unsigned int startAddr)
{
    int addr = startAddr + p / 7;
    freeBlock(*blk);
    readBlock(blk, addr, 0);
}

void sortMergeJoin(unsigned int Raddr, unsigned int Saddr, unsigned int Rsize, unsigned int Ssize, unsigned int resultAddr)
{
    printf("---------------------\n");
    printf("基于排序的连接操作算法:\n");
    printf("---------------------\n");
    unsigned char *blks[3];
    int SX = -1;
    int SY = -1;
    int RX = -1;
    int RY = -1;
    int count = 0;
    // read first block of S
    readBlock(&blks[0], Saddr, 0);
    getXY(&SX, &SY, blks[0], 0);
    // read first block of R
    readBlock(&blks[1], Raddr, 0);
    getXY(&RX, &RY, blks[1], 0);
    write_blk_addr = resultAddr;
    int addr = -1;
    unsigned int ps = 0, pr = 0;
    for (;;)
    {
        if (SX > RX)
        {
            if (!getNextXY(&RX, &RY, blks[1]))
                if (!getNextBlock(&RX, &RY, &blks[1], Raddr + Rsize))
                    break;
            pr++;
        }
        else if (SX < RX)
        {
            if (!getNextXY(&SX, &SY, blks[0]))
                if (!getNextBlock(&SX, &SY, &blks[0], Saddr + Ssize))
                    break;
            ps++;
        }
        else
        {
            count++;
            fillOutputBlockWith2Item(&addr, SX, SY, RX, RY, &blks[2]);
            for (;;)
            {
                if (!getNextXY(&RX, &RY, blks[1]))
                    if (!getNextBlock(&RX, &RY, &blks[1], Raddr + Rsize))
                    {
                        rollBack(pr, &blks[1], Raddr);
                        getXY(&RX, &RY, blks[1], pr % 7);
                        break;
                    }
                if (SX == RX)
                {
                    count++;
                    fillOutputBlockWith2Item(&addr, SX, SY, RX, RY, &blks[2]);
                }
                else
                {
                    rollBack(pr, &blks[1], Raddr);
                    getXY(&RX, &RY, blks[1], pr % 7);
                    break;
                }
            }
            for (;;)
            {
                if (!getNextXY(&SX, &SY, blks[0]))
                    if (!getNextBlock(&SX, &SY, &blks[0], Saddr + Ssize))
                    {
                        rollBack(ps, &blks[0], Saddr);
                        getXY(&SX, &SY, blks[0], ps % 7);
                        break;
                    }
                if (SX == RX)
                {
                    count++;
                    fillOutputBlockWith2Item(&addr, SX, SY, RX, RY, &blks[2]);
                }
                else
                {
                    rollBack(ps, &blks[0], Saddr);
                    getXY(&SX, &SY, blks[0], ps % 7);
                    break;
                }
            }
            if (!getNextXY(&RX, &RY, blks[1]))
                if (!getNextBlock(&RX, &RY, &blks[1], Raddr + Rsize))
                    break;
            pr++;
            if (!getNextXY(&SX, &SY, blks[0]))
                if (!getNextBlock(&SX, &SY, &blks[0], Saddr + Ssize))
                    break;
            ps++;
        }
    }
    writeBlock(blks[2]);
    for (int i = 0; i < 2; i++)
        freeBlock(blks[i]);
    printf("总共连接%d次。\n", count);
}

/**
 * task 5
 */
void sortIntersection(unsigned int Raddr, unsigned int Saddr, unsigned int Rsize, unsigned int Ssize, unsigned int resultAddr)
{
    printf("-----------------------\n");
    printf("基于排序的集合交操作算法:\n");
    printf("-----------------------\n");
    unsigned char *blks[3];
    int SX = -1;
    int SY = -1;
    int RX = -1;
    int RY = -1;
    int count = 0;
    // read first block of S
    readBlock(&blks[0], Saddr, 0);
    getXY(&SX, &SY, blks[0], 0);
    // read first block of R
    readBlock(&blks[1], Raddr, 0);
    getXY(&RX, &RY, blks[1], 0);
    write_blk_addr = resultAddr;
    int addr = -1;
    unsigned int ps = 0, pr = 0;
    for (;;)
    {
        if (SX > RX)
        {
            if (!getNextXY(&RX, &RY, blks[1]))
                if (!getNextBlock(&RX, &RY, &blks[1], Raddr + Rsize))
                    break;
            pr++;
        }
        else if (SX < RX)
        {
            if (!getNextXY(&SX, &SY, blks[0]))
                if (!getNextBlock(&SX, &SY, &blks[0], Saddr + Ssize))
                    break;
            ps++;
        }
        else
        {
            for (;;)
            {
                if (SX != RX)
                {
                    rollBack(pr, &blks[1], Raddr);
                    getXY(&RX, &RY, blks[1], pr % 7);
                    break;
                }
                if (SY == RY)
                {
                    count++;
                    printf("(%d, %d)\n", SX, SY);
                    fillOutputBlockWith1Item(&addr, SX, SY, &blks[2]);
                    rollBack(pr, &blks[1], Raddr);
                    getXY(&RX, &RY, blks[1], pr % 7);
                    break;
                }
                if (!getNextXY(&RX, &RY, blks[1]))
                    if (!getNextBlock(&RX, &RY, &blks[1], Raddr + Rsize))
                    {
                        rollBack(pr, &blks[1], Raddr);
                        getXY(&RX, &RY, blks[1], pr % 7);
                        break;
                    }
            }
            if (!getNextXY(&SX, &SY, blks[0]))
                if (!getNextBlock(&SX, &SY, &blks[0], Saddr + Ssize))
                    break;
            ps++;
            for (;;)
            {
                if (SX != RX)
                {
                    rollBack(ps, &blks[0], Saddr);
                    getXY(&SX, &SY, blks[0], ps % 7);
                    break;
                }
                if (SY == RY)
                {
                    count++;
                    printf("(%d, %d)\n", RX, RY);
                    fillOutputBlockWith1Item(&addr, RX, RY, &blks[2]);
                    rollBack(ps, &blks[0], Saddr);
                    getXY(&SX, &SY, blks[0], ps % 7);
                    break;
                }
                if (!getNextXY(&SX, &SY, blks[0]))
                    if (!getNextBlock(&SX, &SY, &blks[0], Saddr + Ssize))
                    {
                        rollBack(ps, &blks[0], Saddr);
                        getXY(&SX, &SY, blks[0], ps % 7);
                        break;
                    }
            }
            if (!getNextXY(&RX, &RY, blks[1]))
                if (!getNextBlock(&RX, &RY, &blks[1], Raddr + Rsize))
                    break;
            pr++;
        }
    }
    writeBlock(blks[2]);
    for (int i = 0; i < 2; i++)
        freeBlock(blks[i]);
    printf("S和R的交集有%d个元组。\n", count);
}

int main(int argc, char **argv)
{
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // task 1
    write_blk_addr = 100;
    linearSelect();

    // task 2
    write_blk_addr = 200;
    TPMMS(1, 16);

    write_blk_addr = 400;
    TPMMS(17, 48);

    // task 3
    printf("------------------\n");
    printf("构建关系R的索引文件:\n");
    printf("------------------\n");
    initIndex(8, 2, 300, 150); // init R index

    printf("------------------\n");
    printf("构建关系S的索引文件:\n");
    printf("------------------\n");
    initIndex(16, 2, 500, 160); // init S index

    buf.numIO = 0;
    indexBasedSelect(160, 2, 3, 600);

    // task 4
    sortMergeJoin(300, 500, 16, 32, 650);

    // task 5
    sortIntersection(300, 500, 16, 32, 800);

    // free
    freeBuffer(&buf);

    return 0;
}
