#include "kernel/sysinfo.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main()
{
    struct sysinfo info;
    if(sysinfo(&info) < 0){
        printf("sysinfo failed\n");
        exit(1);
    }
    printf("Free Memory: %ld bytes\n", info.freemem);
    printf("Number of Processes: %ld\n", info.nproc);
    printf("Number of Open Files: %ld\n", info.nopenfiles);
    exit(0);
}