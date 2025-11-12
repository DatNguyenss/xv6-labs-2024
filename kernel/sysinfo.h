#ifndef _SYSINFO_H_
#define _SYSINFO_H_
#include "types.h"
struct sysinfo {
    uint64 freemem;
    uint64 nproc;
    uint64 nopenfiles;
};
#endif // _SYSINFO_H_