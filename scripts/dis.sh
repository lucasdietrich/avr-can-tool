# sudo apt-get install gcc-avr binutils-avr avr-libc

avr-objdump -S .pio/build/CanMonitorToolTx/firmware.elf > ./tmp/dis.s
avr-readelf -a .pio/build/CanMonitorToolTx/firmware.elf > ./tmp/readelf.txt
avr-nm .pio/build/CanMonitorToolTx/firmware.elf > ./tmp/symbols.txt