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

# Import your newly compiled SimObject and the CoherentXBar interconnect
from m5.objects import AmxAccl, CoherentXBar

# Standard libraries
from pathlib import Path
import m5.debug

'''
Usage: in root directory run
$ ./gem5.debug amx/tb.py
'''

# Define the path to your compiled test binary
binary_path = Path("amx-workloads/load_test")

# Setup Cache Hierarchy (64 KiB L1 Instruction and Data caches)
cache_hierarchy = PrivateL1CacheHierarchy(
    l1d_size="64KiB",
    l1i_size="64KiB",
)

# Setup Memory Configuration (8 GiB DDR4 RAM using base-2 binary formatting)
memory = SingleChannelDDR4_2400("8GiB")

# Setup the processor (Single-core Timing-accurate C++ execution engine)
# (CPUTypes.ATOMIC is faster for purely functional tests, but TIMING is better if you need cycle counts)
processor = SimpleProcessor(
    cpu_type=CPUTypes.TIMING,  # In-order detailed timing processor model
    num_cores=1,
    isa=ISA.X86
)

# -------------------------------------------------------------------------
# Attach the AMX Accelerator and Intercept the Cache Connection Port
# -------------------------------------------------------------------------
# We iterate through the cores and attach our accelerator SimObject.
# To allow both the CPU and AMX to share the single L1 cache port, we create
# a CoherentXBar to multiplex the traffic safely.
for core in processor.cores:
    # 1. Attach the accelerator SimObject as a structural parameter of the BaseCPU
    core.core.amx_accl = AmxAccl()

    # 2. Save a reference to the standard library's original connection method
    orig_connect_dcache = core.connect_dcache

    # 3. Create our extension wrapper, multiplexing ports via a new local crossbar
    def extended_connect_dcache(port, core_wrapper=core):
        # Instantiate a coherent crossbar bus dedicated to this core
        core_wrapper.amx_l1_xbar = CoherentXBar()

        # Explicitly set all the required hardware latencies (in clock cycles)
        core_wrapper.amx_l1_xbar.forward_latency = 1
        core_wrapper.amx_l1_xbar.response_latency = 1
        core_wrapper.amx_l1_xbar.frontend_latency = 1
        # FIX: Added missing snoop latency parameter
        core_wrapper.amx_l1_xbar.snoop_response_latency = 1
        core_wrapper.amx_l1_xbar.width = 64  # Match a full 64-byte cache line width

        # Connect the downstream side of our crossbar to the L1 Cache port
        core_wrapper.amx_l1_xbar.mem_side_ports = port

        # Connect the original CPU data port to the crossbar's upstream vector ports
        orig_connect_dcache(core_wrapper.amx_l1_xbar.cpu_side_ports)

        # Connect our AMX accelerator master port to the crossbar's upstream vector ports
        core_wrapper.core.amx_accl.mem_side = core_wrapper.amx_l1_xbar.cpu_side_ports

    # 4. Use object.__setattr__ to bypass the strict SimObject parameter check
    object.__setattr__(core, 'connect_dcache', extended_connect_dcache)
# -------------------------------------------------------------------------

# Setup the board (SimpleBoard handles the plumbing for SE mode environment)
board = SimpleBoard(
    clk_freq="1GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

# Setup Workload binary to execute inside the simulator
board.set_se_binary_workload(
    binary=BinaryResource(
        local_path=binary_path.as_posix()
    )
)

# -------------------------------------------------------------------------
# Define Region of Interest (ROI) Handlers
# -------------------------------------------------------------------------


def workbegin_handler():
    print("\n--- Start of AMX ROI (workbegin) ---\n")

    # Enable ExecAll here to trace instructions ONLY in your region of interest
    # This prevents the terminal from being flooded with standard C-library setup instructions.
    # m5.debug.flags["ExecAll"].enable()
    # m5.debug.flags["Cache"].enable()
    m5.debug.flags["PseudoInst"].enable()
    # Enable our custom AMX debug flag to see our DPRINTFs and incoming hex cache values
    m5.debug.flags["AMX"].enable()

    yield False  # Yielding False tells the simulator to resume execution immediately


def workend_handler():
    print("\n--- End of AMX ROI (workend) ---\n")

    # Turn off tracing once your target calculation finishes
    # m5.debug.flags["ExecAll"].disable()
    # m5.debug.flags["Cache"].disable()
    m5.debug.flags["PseudoInst"].disable()
    # m5.debug.flags["AMX"].disable()

    yield False  # Yielding False tells the simulator to resume execution immediately
# -------------------------------------------------------------------------


# Setup and Run Simulator with exit hooks mapped
simulator = Simulator(
    board=board,
    on_exit_event={
        ExitEvent.WORKBEGIN: workbegin_handler(),
        ExitEvent.WORKEND: workend_handler()
    }
)

print(f"Starting SE Simulation for: {binary_path.name}")
simulator.run()
print("Simulation Done")
