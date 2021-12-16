# sudo apt-get install gcc-avr binutils-avr avr-libc

avr-objdump -S .pio/build/caniot-lib-demo/firmware.elf > ./tmp/dis.s
avr-readelf -a .pio/build/caniot-lib-demo/firmware.elf > ./tmp/readelf.txt
avr-nm .pio/build/caniot-lib-demo/firmware.elf > ./tmp/symbols.txt