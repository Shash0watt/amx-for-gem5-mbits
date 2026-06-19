#include <stdio.h>
#include <cstdint>
#include <cstddef>

#include <gem5/m5ops.h> // for gem5 magic ops

int main()
{
    // setup the variables in cpp
    uint64_t dest_tile = 0;

    int data[4] = {7, 8, 9, 10};
    int *data_ptr = data;
    uint64_t src_mem = reinterpret_cast<uint64_t>(data_ptr);

    size_t stride = 64;

    // test the m5op
    m5_work_begin(0, 0);

    // amx_tile_load(uint64_t dest_tile, uint64_t src_mem, size_t stride);
    amx_tile_loadd(dest_tile, src_mem, stride);

    m5_work_end(0, 0);
    // stop the debug messages

    return 0;
}