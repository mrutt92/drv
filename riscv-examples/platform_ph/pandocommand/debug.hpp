// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#ifndef PANDOCOMMAND_DEBUG_HPP
#define PANDOCOMMAND_DEBUG_HPP
#include <iostream>

#ifdef PANDOCOMMAND_DEBUG
#define cmd_dbg(fmt, ...)                                               \
    do {printf("[COMMAND PROCESSOR DEBUG]: " fmt, ##__VA_ARGS__);} while (0)
#else
#define cmd_dbg(fmt, ...)                                               \
    do {} while (0)
#endif

#ifdef PANDOCOMMAND_DEBUG
#define CMD_DBG(stmt)                           \
    do { std::cout << stmt; } while (0)
#else
#define CMD_DBG(stmt)                           \
    do {} while (0)
#endif

#define CMD_FMT_ADDR(addr)                     \
    "0x" << std::hex << addr << std::dec

#endif // PANDOCOMMAND_DEBUG_HPP
