#ifndef __AMX_ACCL_HH__
#define __AMX_ACCL_HH__

#include "params/AmxAccl.hh"
#include "cpu/thread_context.hh"

// #include "sim/sim_object.hh" // swap out since we are using a clocked object
#include "sim/clocked_object.hh" 

#include "mem/port.hh"    // Required for RequestPort definition
#include "mem/packet.hh"  // Required for PacketPtr usage

namespace gem5
{

class AmxAccl : public ClockedObject
{
  private:
    class AmxMemPort : public RequestPort
    {
      private:
        AmxAccl *owner; // pointer to parent accelerator
        static constexpr int MAX_ROWS = 16;
        static constexpr int MAX_COLS_BYTES = 64;
        static constexpr int NUM_TILES = 8;

        struct TileCfg {
            uint8_t palette_id;
            uint8_t start_row;
            uint8_t reserved_0[14];
            uint16_t colsb[16];
            uint8_t rows[16];
        };

        struct TileReg {
            uint16_t rows;
            uint16_t colbytes;
            int8_t data[MAX_ROWS][MAX_COLS_BYTES];
        };

        // internal registers for AMX
        TileCfg currentCfg;           // Global config state register
        TileReg tiles[NUM_TILES];     // Matrix register file (TMM0 - TMM7)

      public:
        AmxMemPort(const std::string &name, AmxAccl *owner) :
            RequestPort(name), owner(owner) {}

        void recvReqRetry() override;
        bool recvTimingResp(PacketPtr pkt) override;

        void recvRangeChange() override {}
    };
    // Renamed member variable to amxMemPort
    AmxMemPort amxMemPort;

  public:
    AmxAccl(const AmxAcclParams &p);
    void startup() override;
    // void startAmxLoad(uint64_t dest_tile, uint64_t src_mem, size_t stride); // for the m5op to use
    void startAmxLoad(ThreadContext *tc, uint64_t dest_tile, uint64_t src_mem, size_t stride);
    Port &amp;getPort(const std::string &amp;if_name, PortID idx = InvalidPortID) override;
};

} // namespace gem5

#endif // __AMX_ACCL_HH__