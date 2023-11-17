# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

########################################################################################################
# Diagram of this model:                                                                               #
# https://docs.google.com/presentation/d/1FnrAjOXJKo5vKgo7IkuSD7QT15aDAmJi5Pts6IQhkX8/edit?usp=sharing #
########################################################################################################
from drv import *
from drv_memory import *
from drv_tile import *

# for drvr we set the core clock to 1GHz
SYSCONFIG["sys_core_clock"] = "1GHz"

print("""
PANDOHammerDrvR:
  program = {}
""".format(
    arguments.program
))

MakeTile = lambda id : DrvRTile(id)

# build the tiles
tiles = []
CORES = SYSCONFIG["sys_pod_cores"]
for i in range(CORES):
    tiles.append(MakeTile(i))

tiles[0].markAsLoader()

# build the shared memory
POD_L2_BANKS = SYSCONFIG["sys_pod_l2_banks"]
l2_banks = []
for i in range(POD_L2_BANKS):
    l2_banks.append(L2MemoryBank(i))

# build the main memory banks
POD_MAINMEM_BANKS = SYSCONFIG["sys_pod_dram_ports"]
mainmem_banks = []
for i in range(POD_MAINMEM_BANKS):
    mainmem_banks.append(MainMemoryBank(i))

# build the network crossbar
chiprtr = sst.Component("chiprtr", "merlin.hr_router")
chiprtr.addParams({
    # semantics parameters
    "id" : CHIPRTR_ID,
    "num_ports" : CORES+POD_L2_BANKS+POD_MAINMEM_BANKS+(1 if arguments.with_command_processor else 0),
    "topology" : "merlin.singlerouter",
    # performance models
    "xbar_bw" : "256GB/s",
    "link_bw" : "256GB/s",
    "flit_size" : "8B",
    "input_buf_size" : "1KB",
    "output_buf_size" : "1KB",
})
chiprtr.setSubComponent("topology","merlin.singlerouter")

# wire up the tiles network
for (i, tile) in enumerate(tiles):
    bridge = sst.Component("bridge_%d" % i, "merlin.Bridge")
    bridge.addParams({
        "translator" : "memHierarchy.MemNetBridge",
        "debug" : 1,
        "debug_level" : 10,
        "network_bw" : "256GB/s",
    })
    tile_bridge_link = sst.Link("tile_bridge_link_%d" % i)
    tile_bridge_link.connect(
        (bridge, "network0", "1ns"),
        (tile.tile_rtr, "port2", "1ns")
    )
    bridge_chiprtr_link = sst.Link("bridge_chiprtr_link_%d" % i)
    bridge_chiprtr_link.connect(
        (bridge, "network1", "1ns"),
        (chiprtr, "port%d" % i, "1ns")
    )

# wire up the shared memory
base_l2_bankno = len(tiles)
for (i, l2_bank) in enumerate(l2_banks):
    bridge = sst.Component("bridge_%d" % (base_l2_bankno+i), "merlin.Bridge")
    bridge.addParams({
        "translator" : "memHierarchy.MemNetBridge",
        "debug" : 1,
        "debug_level" : 10,
        "network_bw" : "256GB/s",
        })
    l2_bank_bridge_link = sst.Link("l2bank_bridge_link_%d" % i)
    l2_bank_bridge_link.connect(
        (bridge, "network0", "1ns"),
        (l2_bank.mem_rtr, "port1", "1ns")
    )
    bridge_chiprtr_link = sst.Link("bridge_chip_memrtr_link_%d" %i)
    bridge_chiprtr_link.connect(
        (bridge, "network1", "1ns"),
        (chiprtr, "port%d" % (base_l2_bankno+i), "1ns")
    )
        
# wire up the main memory
base_mainmem_bankno = base_l2_bankno + len(l2_banks)
for (i, mainmem_bank) in enumerate(mainmem_banks):
    bridge = sst.Component("mainmem_bridge_%d" % i, "merlin.Bridge")
    bridge.addParams({
        "translator" : "memHierarchy.MemNetBridge",
        "debug" : 1,
        "debug_level" : 10,
        "network_bw" : "256GB/s",
    })
    mainmem_bank_bridge_link = sst.Link("mainmem_bank_bridge_link_%d" % i)
    mainmem_bank_bridge_link.connect(
        (bridge, "network0", "1ns"),
        (mainmem_bank.mem_rtr, "port1", "1ns")
    )
    bridge_chiprtr_link = sst.Link("bridge_chip_mainmem_memrtr_link_%d" %i)
    bridge_chiprtr_link.connect(
        (bridge, "network1", "1ns"),
        (chiprtr, "port%d" % (base_mainmem_bankno+i), "1ns")
    )

# wire up the command processor
if arguments.with_command_processor:
    command_processor = CommandProcessor()
    chiprtr_command_processor_link = sst.Link("chiprtr_command_processor_link_pxn%d" % 0)
    chiprtr_command_processor_link.connect(
        (chiprtr, "port%d" % (CORES+POD_L2_BANKS+POD_MAINMEM_BANKS), "1ns"),
        (command_processor.core_nic, "port", "1ns")
    )
