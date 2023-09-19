#SST_RISCV_DIR := $(shell git rev-parse --show-toplevel)
#include $(SST_RISCV_DIR)/mk/config.mk

CXX := riscv64-unknown-elf-g++
CC  := riscv64-unknown-elf-gcc

CSOURCE := $(wildcard *.c)
COBJECT := $(patsubst %.c,%.o,$(CSOURCE))
CXXSOURCE := $(wildcard *.cpp)
CXXOBJECT := $(patsubst %.cpp,%.o,$(CXXSOURCE))
ASMSOURCE := $(wildcard *.S)
ASMOBJECT := $(patsubst %.S,%.o,$(ASMSOURCE))

ARCH:=rv64imafd
ABI:=lp64d

CXXFLAGS := -O2 -nostdlib -march=$(ARCH) -mabi=$(ABI)
CFLAGS := -O2 -nostdlib -march=$(ARCH) -mabi=$(ABI)

$(ASMOBJECT): %.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

$(COBJECT): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(CXXOBJECT): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

RISCVS := li.riscv
RISCVS += beq.riscv
RISCVS += lsu.riscv
RISCVS += shift.riscv
RISCVS += mul.riscv
RISCVS += fib.riscv
RISCVS += exit.riscv
RISCVS += write.riscv
RISCVS += read.riscv
RISCVS += simple.riscv
RISCVS += malloc.riscv
RISCVS += puts.riscv
RISCVS += snprintf.riscv
RISCVS += print_int.riscv
RISCVS += add.riscv
RISCVS += open.riscv
RISCVS += amoswap.riscv

li.riscv: li.o
beq.riscv: beq.o
lsu.riscv: lsu.o
shift.riscv: shift.o
mul.riscv: mul.o
fib.riscv: fib-main.o fib.o
write.riscv: write.o
read.riscv: read.o
simple.riscv: simple.o simple-main.o crt.o
malloc.riscv: malloc.o malloc-main.o
puts.riscv: puts.o
snprintf.riscv: snprintf-main.o snprintf-test.o
print_int.riscv: print_int.o
exit.riscv: exit.o
add.riscv: add.o
open.riscv: open.o
amoswap.riscv: amoswap.o

WITH_STDLIB += write.riscv write.o
WITH_STDLIB += fib.riscv fib-main.o fib.o
WITH_STDLIB += malloc.riscv malloc.o malloc-main.o
WITH_STDLIB += puts.riscv puts.o
WITH_STDLIB += snprintf.riscv snprintf-main.o snprintf-test.o
WITH_STDLIB += print_int.riscv print_int.o
WITH_STDLIB += read.riscv read.o
WITH_STDLIB += open.riscv open.o
WITH_STDLIB += amoswap.riscv amoswap.o

$(WITH_STDLIB): CFLAGS := $(filter-out -nostdlib,$(CFLAGS))
$(WITH_STDLIB): CXXFLAGS := $(filter-out -nostdlib,$(CXXFLAGS))

$(RISCVS): %.riscv:
	$(CXX) $(CXXFLAGS) -o $@ $(filter %.o,$^)

.PHONY: clean all

all: $(RISCVS)

clean:
	rm -f *.o *.riscv


