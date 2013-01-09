### Start a remote debugging session ###

file build/maple_native.elf
set prompt (leaf) 
set logging file sdio-gdb.log
set logging overwrite on
set logging on
set mem inaccessible-by-default off
target extended-remote /dev/cu.usbmodemDDE4C3C1
mon jtag_scan
attach 1
info target
#break HardwareSDIO.cpp:132
load
display SDIO->regs->RESP4
display SDIO->regs->RESP3
display SDIO->regs->RESP2
display SDIO->regs->RESP1
display SDIO->regs->RESPCMD
display SDIO->regs->ARG
display SDIO->regs->CMD
display SDIO->regs->STA