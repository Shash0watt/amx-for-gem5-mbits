#ifndef __AMX_ACCL_HH__
#define __AMX_ACCL_HH__

#include "params/AmxAccl.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class AmxAccl : public SimObject
{
  private:
    void processEvent();
    EventFunctionWrapper event;

  
  public:
    AmxAccl(const AmxAcclParams &p);

    void startup() override;

    void startAmxLoad(uint64_t dest_tile, uint64_t src_mem, size_t stride);
};

} // namespace gem5

#endif // __AMX_ACCL_HH_