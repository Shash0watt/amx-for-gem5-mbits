
"""Simple config/run script for the HelloObject

This is probably the simplest gem5 config file you can possibly create.
It creates a Root object and one *very* simple SimObject and simulates the
system. Since there are no events, this "simulation" should finish immediately

"""

# import the m5 (gem5) library created when gem5 is built
import m5

# import all of the SimObjects
from m5.objects import *

# set up the root SimObject and start the simulation
root = Root(full_system=False)

# Create an instantiation of the simobject you created
root.AMX = AmxAccl()

# instantiate all of the objects we've created above
m5.instantiate()

print(f"Beginning simulation!")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
