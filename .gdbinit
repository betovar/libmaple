### Start a remote debugging session ###

file build/maple_native.elf
set prompt (leaf) 
set width 80
set logging file sdio-gdb.log
set logging overwrite on
set logging on
set mem inaccessible-by-default off
target extended-remote /dev/cu.usbmodemDDE4C3C1
mon jtag_scan
attach 1
info target
break HardwareSDIO::getCID
break HardwareSDIO::getCSD
break HardwareSDIO::end
load
display/x SDIO->regs->RESP4
display/x SDIO->regs->RESP3
display/x SDIO->regs->RESP2
display/x SDIO->regs->RESP1
display/x SDIO->regs->ARG
display/x SDIO->regs->CMD
display/x SDIO->regs->RESPCMD
display/x SDIO->regs->STA
