// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#ifndef PANDOHAMMER_CPUINFO_H
#define PANDOHAMMER_CPUINFO_H
#include <stdint.h>
#include <pandohammer/mcsr.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __stringify
#define __stringify_1(x) #x
#define __stringify(x) __stringify_1(x)
#endif

/**
 * thread id wrt my core
 */ 
inline int myThreadId()
{
    int64_t tid;
    asm volatile ("csrr %0, mhartid" : "=r"(tid));
    return (int)tid;
}

/**
 * core id wrt my pod
 */
inline int myCoreId()
{
    int64_t cid;
    asm volatile ("csrr %0, " __stringify(MCSR_MCOREID) : "=r"(cid));
    return (int)cid;
}


/**
 * return a core's x  w.r.t my pod
 */
inline int coreXFromId(int core)
{
    return core & 7;
}

/**
 * return a core's y  w.r.t my pod
 */
inline int coreYFromId(int core)
{
    return (core >> 3) & 7;
}

/**
 * return a core's id from its x y
 */
inline int coreIdFromXY(int x, int y)
{
    return x + (y << 3);
}

/**
 * return a core's x w.r.t my pod
 */
inline int myCoreX()
{
    return coreXFromId(myCoreId());
}

/**
 * return a core's y w.r.t my pod
 */
inline int myCoreY()
{
    return coreYFromId(myCoreId());
}
    
/**
 * pod id wrt my pxn
 */
inline int myPodId()
{
    int64_t pid;
    asm volatile ("csrr %0, " __stringify(MCSR_MPODID) : "=r"(pid));
    return (int)pid;
}

/**
 * pxn id
 */
inline int myPXNId()
{
    int64_t xid;
    asm volatile ("csrr %0, " __stringify(MCSR_MPXNID) : "=r"(xid));
    return (int)xid;
}

/**
 * number of hardware threads on my core
 */
inline int myCoreThreads()
{
    int64_t harts;
    asm volatile ("csrr %0, " __stringify(MCSR_MCOREHARTS) : "=r"(harts));
    return (int)harts;
}

/**
 * number of hardware threads in a core
 */
inline int numCoreThreads()
{
    return myCoreThreads();
}

/**
 * number of pxns in system
 */
inline int numPXN()
{
    int64_t num;
    asm volatile ("csrr %0, " __stringify(MCSR_MNUMPXN) : "=r"(num));
    return (int)num;
}

/**
 * an alias for numPXN (makes copy-pasting easier)
 */
inline int numPXNs()
{
    return numPXN();
}
    
/**
 * number of cores in a pod
 */
inline int numPodCores()
{
    int64_t cores;
    asm volatile ("csrr %0, " __stringify(MCSR_MPODCORES) : "=r"(cores));
    return (int)cores;
}

/**
 * number of pods in a pxn
 */
inline int numPXNPods()
{
    int64_t pods;
    asm volatile ("csrr %0, " __stringify(MCSR_MPXNPODS) : "=r"(pods));
    return (int)pods;
}

/**
 * size of l1sp in bytes
 */
inline uint64_t coreL1SPSize() {
    uint64_t l1sp_size;
    asm volatile ("csrr %0, " __stringify(MCSR_MCOREL1SPSIZE) : "=r"(l1sp_size));
    return l1sp_size;
}

/**
 * size of l2sp in bytes
 */
inline uint64_t podL2SPSize() {
    uint64_t l2sp_size;
    asm volatile ("csrr %0, " __stringify(MCSR_MPODL2SPSIZE) : "=r"(l2sp_size));
    return l2sp_size;
}

/**
 * size of pxn's dram in bytes
 */
inline uint64_t pxnDRAMSize() {
    uint64_t dram_size;
    asm volatile ("csrr %0, " __stringify(MCSR_MPXNDRAMSIZE) : "=r"(dram_size));
    return dram_size;
}

/**
 * get the current cycle count
 */
inline uint64_t cycle() {
    uint64_t cycle;
    asm volatile ("rdcycle %0" : "=r"(cycle));
    return cycle;
}

/**
 * wait for a number of cycles
 */
inline void wait_cycles(uint64_t cycles) {
    asm volatile ("csrw " __stringify(MCSR_MWAIT) ", %0" : : "r"(cycles));
    return;
}

#ifdef __cplusplus
}
#endif
#endif
