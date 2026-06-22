from m5.params import *
from m5.SimObject import SimObject


class AmxAccl(SimObject):
    type = "AmxAccl"
    cxx_header = "amx/amx_accl.hh"
    cxx_class = "gem5::AmxAccl"
