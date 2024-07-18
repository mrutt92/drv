MEMORY
{
        L1SP_VMA (rw)  : ORIGIN = 0x00000000, LENGTH = 0x20000000
        L2SP_VMA (rw)  : ORIGIN = 0x20000000, LENGTH = 0x20000000
        DRAM_VMA (rwx) : ORIGIN = 0x40000000, LENGTH = 0x40000000
}

SECTIONS
{
.l1sp :
{
        *(.l1sp.interrupt)
        *(.l1sp)
        *(.l1sp.*)
        . = ALIGN(16);
} > L1SP_VMA


.l2sp :
{
        *(.l2sp.interrupt)
        *(.l2sp)
        *(.l2sp.*)
        . = ALIGN(16);
}> L2SP_VMA


.text.dram :
{
*(.text.interrupt)
*(.crtbegin)
*(.text)
*(.text.startup)
*(.text.*)
. = ALIGN(16);
} > DRAM_VMA

.eh_frame.dram :
{
*(.eh_frame)
*(.eh_frame*)
. = ALIGN(16);
} > DRAM_VMA

.rodata.dram :
{
*(.rodata)
*(.rodata.*)
*(.srodata.cst16)
*(.srodata.cst8)
*(.srodata.cst4)
*(.srodata.cst2)
*(.srodata)
. = ALIGN(16);
} > DRAM_VMA

.data.dram :
{
*(.dram)
*(.dram.*)
*(.data)
*(.data*)
. = ALIGN(16);
} > DRAM_VMA

.sdata.dram :
{
*(.sdata)
*(.sdata.*)
*(.sdata*)
*(.sdata*.gnu.linkonce.s.*)
*(.sbss)
*(.sbss*)
*(.gnu.linkonce.sb.*)
*(.scommon)
. = ALIGN(16);
} > DRAM_VMA

.bss.dram :
{
*(.bss)
*(.bss*)
. = ALIGN(16);
} > DRAM_VMA


__global_pointer$ = 0x40000000;
_end = .;
end = .;
_edata = .;
ENTRY(_start)
}

