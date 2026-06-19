#include <stdio.h>
#include <unistd.h>
#include <gem5/m5ops.h> // for annotation in gem5

// compile changes in gem5 by running 'scons build/{ISA}/gem5.{variant} -j {cpus}'
// complile by running 'make' in terminal

int main(void) {
    volatile int data[4] = {25, 50, 75, 100};
    int *data_ptr = data;
    uint64_t ptr = (uint64_t)data_ptr;
    int stride = 64;
    m5_work_begin(0,0); // mark the start of the workload

    // amx_tile_loadd(uint64_t tile_num, uint64_t ptr, size_t stride
    amx_tile_loadd(0, ptr, stride); // this in theory should be in mem now

    m5_work_end(0,0); // mark the end of the workload

    // printf("--- print whatever you want to see as an output here, without any debugging ---\n");
    return 0;
}
