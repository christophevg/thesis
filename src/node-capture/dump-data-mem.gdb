set width 0
set height 0
set verbose off

target remote localhost:4242
dump binary memory dump.bin 0x800100 0x800124
