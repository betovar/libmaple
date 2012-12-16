### Start a remote debugging session ###
target extended-remote /dev/tty.usbmodemDDE4C3C1
mon jtag_scan
attach 1
set prompt (maple) 
source sdio.gdb