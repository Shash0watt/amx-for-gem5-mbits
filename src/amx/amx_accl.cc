#include "amx/amx_accl.hh"

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "arch/generic/mmu.hh"
#include "base/trace.hh"
#include "cpu/base.hh"
#include "debug/AMX.hh"
#include "params/AmxAccl.hh"

namespace gem5
{

AmxAccl::AmxAccl(const Params &params)
    : ClockedObject(params), cpu(nullptr), currentCfg{}
{
    // initialize tiles array to zero
    for (int i = 0; i < NUM_TILES; i++) {
        tiles[i] = {};
    }

    // set up default configuration values
    // // for debugging and so that we don't need the tile config instruction
    // right now
    currentCfg.palette_id = 1;
    currentCfg.start_row = 0;

    // set default row and column bounds for all tiles
    for (int i = 0; i < NUM_TILES; i++) {
        currentCfg.rows[i] = MAX_ROWS;
        currentCfg.colsb[i] = MAX_COLS_BYTES;
    }

    DPRINTF(AMX, "created the amx object\n");
}

void
AmxAccl::setCPU(BaseCPU *_cpu)
{
    cpu = _cpu;
    DPRINTF(AMX, "parent cpu is set to %s\n", cpu->name());
}

void
AmxAccl::handleMemResponse(PacketPtr pkt)
{
    DPRINTF(AMX, "amx: handleMemResponse called for packet at paddr 0x%lx\n",
            pkt->getAddr());

    // inline lambda function to delete packets without repeating code
    auto dropPacket = [](PacketPtr p) {
        if (p->senderState) {
            delete p->popSenderState();
        }
        delete p;
    };

    // check if the memory request failed
    if (pkt->isError()) {
        DPRINTF(AMX, "packet returned with an error status\n");
        dropPacket(pkt);
        return;
    }

    // check if the packet is empty
    if (!pkt->hasData()) {
        DPRINTF(AMX, "packet arrived safely but contains no data payload\n");
        dropPacket(pkt);
        return;
    }

    // extract and verify the tracking state from the packet
    auto *state = dynamic_cast<AmxSenderState *>(pkt->popSenderState());
    panic_if(
        !state,
        "amx response packet arrived missing its tracking senderstate token!");

    // convert raw packet bytes into a readable int8 string for debugging
    auto *data_ptr = reinterpret_cast<int8_t *>(pkt->getPtr<uint8_t>());
    std::string int8_output;
    for (int i = 0; i < pkt->getSize(); i++) {
        int8_output += std::to_string(data_ptr[i]) + " ";
    }

    DPRINTF(AMX, "data loaded into matrix (as int8): [ %s]\n",
            int8_output.c_str());

    // clean up allocated memory
    delete state;
    delete pkt;
}

void
AmxAccl::startup()
{ DPRINTF(AMX, "amx object startup completed\n"); }

void
AmxAccl::startAmxLoad(ThreadContext *tc, uint64_t dest_tile, uint64_t src_mem,
                      std::size_t stride)
{
    DPRINTF(AMX, "received amxload. dest: %llu, src: %llu, stride: %lu\n",
            dest_tile, src_mem, stride);

    // stop early if there is no cpu pointer to avoid crashes or wasting
    // resources
    if (!cpu) {
        DPRINTF(AMX, "warning: cpu pointer is null in startAmxLoad! dropping "
                     "request.\n");
        return;
    }

    // get the cache port once and reuse it to avoid multiple dynamic casts
    auto &dcache_port = dynamic_cast<RequestPort &>(cpu->getDataPort());
    DPRINTF(AMX, "accessing cpu dcache port: %s\n", dcache_port.name());

    // align the memory address to a standard 64-byte cache line
    constexpr int CACHE_LINE_SIZE = 64;
    uint64_t aligned_src_mem = src_mem & ~(CACHE_LINE_SIZE - 1);

    // build the memory request object
    RequestPtr req =
        std::make_shared<Request>(aligned_src_mem, CACHE_LINE_SIZE, 0,
                                  tc->getCpuPtr()->dataRequestorId(),
                                  tc->pcState().instAddr(), tc->contextId());

    // translate the virtual memory address to a physical address using the mmu
    Fault fault = tc->getMMUPtr()->translateFunctional(req, tc, BaseMMU::Read);

    // stop if the address translation fails
    if (fault != NoFault) {
        DPRINTF(AMX, "translation failed for virtual address 0x%lx\n",
                src_mem);
        return;
    }

    // allocate a new memory packet and attach tracking state to it
    PacketPtr pkt = new Packet(req, MemCmd::ReadReq);
    pkt->allocate();
    pkt->pushSenderState(new AmxSenderState(dest_tile, 0));

    // try to send the packet through the cache port
    if (dcache_port.sendTimingReq(pkt)) {
        DPRINTF(AMX,
                "timing read request successfully sent for physical address "
                "0x%lx\n",
                req->getPaddr());
    } else {
        DPRINTF(AMX, "cpu dcache port rejected the timing request because it "
                     "is busy\n");
        // clean up the packet immediately if it was rejected
        delete pkt->popSenderState();
        delete pkt;
    }
}

} // namespace gem5
