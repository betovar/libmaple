### Start a remote debugging session ###
file build/maple_native.elf
target extended-remote /dev/tty.usbmodemDDE4C3C1
info target
mon jtag_scan
attach 1
set prompt (maple) 
break HardwareSDIO::getCID
break HardwareSDIO::getCSD
display SDMC
load
#run
#info registers