# sudo apt-get install gcc-avr binutils-avr avr-libc

avr-objdump -S .pio/build/pro16MHzatmega328/firmware.elf > ./tmp/dis.s
avr-readelf -a .pio/build/pro16MHzatmega328/firmware.elf > ./tmp/readelf.txt
avr-nm .pio/build/pro16MHzatmega328/firmware.elf > ./tmp/symbols.txt