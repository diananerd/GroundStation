# Platzi Ground Station
This is the repo for Platzi Sat Ground Station

### Build firmware
```sh
idf.py build
```

### Merge all binary files to firmware.bin
```sh
esptool.py --chip esp32 merge_bin -o firmware.bin --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0xd000 build/ota_data_initial.bin  0x10000 build/ground-station.bin
```

### Flash firmware with esptool
```sh
esptool.py --chip=esp32 -p /dev/ttyACM0 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x0 firmware.bin
```