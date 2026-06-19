from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.cachehierarchies.ruby.mesi_two_level_cache_hierarchy import (
    MESITwoLevelCacheHierarchy,
)
from gem5.components.memory.single_channel import SingleChannelDDR4_2400
from gem5.components.processors.cpu_types import CPUTypes
from gem5.isas import ISA
from gem5.resources.resource import obtain_resource
from gem5.simulate.simulator import Simulator

# 1. creating a cache hierarchy
# this components represents the two level MESI hierarchy,
# this uses the Ruby memory model
# Ruby is a highly-detailed model with many different coherence protocols
# (specified in a language called "SLICC")
cache_hierarchy = MESITwoLevelCacheHierarchy(
    l1d_size="16KiB",
    l1d_assoc=8,
    l1i_size="16KiB",
    l1i_assoc=6,
    l2_size="256KiB",
    l2_assoc=16,
    num_l2_banks=1,
)

# 2. adding the memory system
# this has a default size of 8gb, but the size could be changed using size="1GiB
memory = SingleChannelDDR4_2400(size="8GiB")

# 3. Create a processsor
# simple processsor has N cores.. all cores are equal and connected to the memory system
# processor = SimpleProcessor(type=CPUTypes.TIMING)
processor = SimpleProcessor(cpu_type=CPUTypes.ATOMIC, isa=ISA.X86, num_cores=2)

# 4. create the board that you will plug everything into
# simple board is SE (syscall emulation)
# this is ISA agnostic
# normally the if you have a ARM proc -> you plug it into a ARM board
board = SimpleBoard(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

# 5. how do I decied what os and tasks should my proc run
# this means get GAP Benchmark Suite (GABPS) for x86
board.set_workload(obtain_resource("x86-gapbs-bfs-run"))
# and run the BFS workload
# this uses the gem5 resources infrastrucutre..

# this is a online collection of resources
# which can be loaded in and used to run the simulation
# check availible resources here : https://resources.gem5.org/

# 6. set up the simulator
# lets to control the simulation (the conductor)
simulator = Simulator(board)
simulator.run()  # the number is the parameter max ticks
