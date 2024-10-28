#ifndef DRV_API_CORE_XY_H
#define DRV_API_CORE_XY_H
#include <DrvAPIBits.hpp>
#include <DrvAPIThread.hpp>

namespace DrvAPI
{

/**
 * return a core's x  w.r.t my pod
 */
inline int coreXFromId(int core) {
    return DrvAPIThread::current()->coreXFromId(core);
}

/**
 * return a core's y  w.r.t my pod
 */
inline int coreYFromId(int core) {
    return DrvAPIThread::current()->coreYFromId(core);
}

/**
 * return a core's id from its x y
 */
inline int coreIdFromXY(int x, int y) {
    return DrvAPIThread::current()->coreIdFromXY(x, y);
}

}
#endif
