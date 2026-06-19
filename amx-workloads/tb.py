# System components
from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.memory.single_channel import SingleChannelDDR4_2400
from gem5.components.cachehierarchies.classic.private_l1_cache_hierarchy import PrivateL1CacheHierarchy

# Simulation components
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import BinaryResource
from gem5.simulate.exit_event import ExitEvent
from gem5.simulate.simulator import Simulator
from gem5.isas import ISA

# Standard libraries
from pathlib import Path
import m5.debug

'''
Usage: in root directory run
$ make
$ ./gem5.debug -rs amx/tb.py
'''

# Define the path to your compiled test binary containing the custom instructions
binary_path = Path("amx/load_test")

# Setup Cache and Memory
cache_hierarchy = PrivateL1CacheHierarchy(
    l1d_size="64kB",
    l1i_size="64kB",
)

memory = SingleChannelDDR4_2400("1GB")

# Setup the processor
# (CPUTypes.ATOMIC is faster for purely functional tests, but TIMING is better if you need cycle counts)
processor = SimpleProcessor(
    cpu_type=CPUTypes.TIMING,  # in order proc
    # cpu_type=CPUTypes.O3, config for Out of Order
    num_cores=1,
    isa=ISA.X86
)

# Setup the board (SimpleBoard is specifically used for SE mode)
board = SimpleBoard(
    clk_freq="1GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

# Setup Workload
board.set_se_binary_workload(
    binary=BinaryResource(
        local_path=binary_path.as_posix()
    )
)


def workbegin_handler():
    print("\n--- Start of AMX instruction ---\n")

    # Enable ExecAll here to trace instructions ONLY in your region of interest
    # This prevents the terminal from being flooded with standard C-library setup instructions.
    m5.debug.flags["ExecAll"].enable()
    m5.debug.flags["Cache"].enable()
    m5.debug.flags["PseudoInst"].enable()

    yield False  # Yielding False tells the simulator to continue running


def workend_handler():
    print("\n--- End of AMX instruction ---\n")

    # Disable tracing once the work is done
    m5.debug.flags["ExecAll"].disable()
    m5.debug.flags["Cache"].disable()
    m5.debug.flags["PseudoInst"].disable()

    yield False  # Yielding False tells the simulator to continue running


# Setup and Run Simulator
simulator = Simulator(
    board=board,
    on_exit_event={
        ExitEvent.WORKBEGIN: workbegin_handler(),
        ExitEvent.WORKEND: workend_handler()
    },
)

print(f"Starting SE Simulation for: {binary_path.name}")
simulator.run()
print("Simulation Done")
