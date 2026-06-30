from m5.params import *
from m5.proxy import *
# from m5.SimObject import SimObject
from m5.objects.ClockedObject import ClockedObject


class AmxAccl(ClockedObject):
    type = "AmxAccl"
    cxx_header = "amx/amx_accl.hh"
    cxx_class = "gem5::AmxAccl"

    # memory side port to ask for cache reads
    mem_side_port = RequestPort(
        "Memory side port to send timing read requests")