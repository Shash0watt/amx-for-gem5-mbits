# define your imports
from gem5.prebuilt.demo.x86_demo_board import X86DemoBoard
from gem5.resources.resource import obtain_resource
from gem5.simulate.simulator import Simulator

# 1. add the (mother)board to the script
board = X86DemoBoard()
# this is a prebuilt board that can be used as is,
# but usually you would like to configure the board to represent
# a real system that you want to simulate

# 2. next we need a workload for the board
board.set_workload(obtain_resource("x86-ubuntu-24.04-boot-no-systemd"))
# the funciton obtain_resource downloads workloads and resources
# For the x86-ubuntu-24.04-boot-no-systemd, it downloads a disk image and kernel, and sets default parameters.

# there are 3 exit events for this workload,
# the simulation can exit and perform other operations on each exit

# 3. to chang the behaviour of an exit event we need to set up an exit event handler
# (ie. to run the simulation for a set number of ticks)
sim = Simulator(board)
sim.run(20_000_000_000)  # 20 billion ticks ~20ms

# 4. to run the simulaton run the command
# - if you haven't built the binary: ./build/ALL/gem5.opt configs/tutorial/part1/simple.py
# - if you have built the binary: gem5 configs/tutorial/part1/simple.py
#       - make a sym link `ln -s build/X86/gem5.opt gem5`
#       - or you need to point to the specific binary. From the
#         ~/gem5 directory, the command should look like this:
#         ./build/X86/gem5.opt configs/tutorial/part1/simple.py)

# also running a the command `telnet localhost [port no.]`
