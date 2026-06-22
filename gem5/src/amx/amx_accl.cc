#include "amx/amx_accl.hh"
#include <iostream>

// Needed for debug flags
#include "base/trace.hh" 
#include "debug/AMX.hh"

namespace gem5
{

AmxAccl::AmxAccl(const AmxAcclParams &params) :
    SimObject(params), 
    event([this]{processEvent();}, name())
{
    // std::cout << "Hello World! From a AmxAccl!" << std::endl;
    DPRINTF(AMX, "Created the AMX object\n");

}

// this is the action that we are doing
void
AmxAccl::processEvent(){
    DPRINTF(AMX, "Doing processEvent()\n");
}

// this is what the system calls when the event starts 
void
AmxAccl::startup()
{
    schedule(event, curTick() + 100);
}

void
AmxAccl::startAmxLoad(uint64_t dest_tile, uint64_t src_mem, size_t stride)
{
    // Fire our custom debug flag to prove the connection works
    DPRINTF(AMX, "Received amxLoadd! Dest: %llu, Src: %llu, Stride: %lu\n", 
            dest_tile, src_mem, stride);
}

} // namespace gem5