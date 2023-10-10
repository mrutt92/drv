#ifndef DRV_API_INFO_H
#define DRV_API_INFO_H
#include <DrvAPIThread.hpp>
#include <DrvAPISysConfig.hpp>

namespace DrvAPI
{

/////////////////////////////////
// Thread-Relative Information //
/////////////////////////////////
/**
 * return my thread id w.r.t my core
 */
inline int myThreadId() {
    return DrvAPIThread::current()->threadId();
}

/**
 * return my core id w.r.t my pod
 */
inline int myCoreId() {
    return DrvAPIThread::current()->coreId();
}

/**
 * return my pod id w.r.t my pxn
 */
inline int myPodId() {
    //DrvAPIThread::current()->podId();
    return 0;
}

/**
 * return my pxn id
 */
inline int myPXNId() {
    //DrvAPIThread::current()->pxnId();
    return 0;
}

inline int myCoreThreads() {
    //DrvAPIThread::current()->coreThreads();
    return 0;
}

//////////////////////
// System Constants //
//////////////////////
inline int numPXNs() {
    return DrvAPISysConfig::Get()->numPXN();
}

inline int numPXNPods() {
    return DrvAPISysConfig::Get()->numPXNPods();
}

inline int numPodCores() {
    return DrvAPISysConfig::Get()->numPodCores();
}

}
#endif
