### Start a remote debugging session ###

file build/maple_native.elf
set prompt (maple) 
set logging file sdio-gdb.log
set logging overwrite on
set logging on
set architecture arm
target extended-remote /dev/tty.usbmodemDDE4C3C1
info target
mon jtag_scan
attach 1
load
break main.cpp:28 #begin()
break HardwareSDIO::idle
break HardwareSDIO::initialization
#info registers