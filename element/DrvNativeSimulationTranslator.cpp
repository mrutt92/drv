#include "DrvNativeSimulationTranslator.hpp"
#include "rv64simtypes/stat.h"

using namespace SST;
using namespace Drv;

std::vector<unsigned char>
DrvNativeSimulationTranslator::nativeToSimulator_stat(const struct stat *i) {
    std::vector<unsigned char> ret(sizeof(rv64sim_stat_t));
    rv64sim_stat_t *o = reinterpret_cast<rv64sim_stat_t *>(&ret[0]);
    o->st_dev = i->st_dev;
    o->st_ino = i->st_ino;
    o->st_mode = i->st_mode;
    o->st_nlink = i->st_nlink;
    o->st_uid = i->st_uid;
    o->st_gid = i->st_gid;
    o->st_rdev = i->st_rdev;
    o->st_size = i->st_size;
    o->st_atim = {};
    o->st_mtim = {};
    o->st_ctim = {};
    o->st_blksize = i->st_blksize;
    o->st_blocks = i->st_blocks;
    return ret;
}

