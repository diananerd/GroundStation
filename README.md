# Platzi Ground Station
This is the repo for Platzi Sat Ground Station

### Build firmware as multiple binary files (default)
```sh
idf.py build
```

### Build firmware and merge binary files to single firmware.bin
Pass the build version as first argument
```sh
./build.sh "1.0.0"
```

### Only merge all binary files to single firmware.bin
```sh
esptool.py --chip esp32 merge_bin -o firmware.bin --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0xd000 build/ota_data_initial.bin  0x10000 build/ground-station.bin
```

### Flash multiple binary files (default)
```sh
idf.py -p /dev/ttyACM0 flash
```

### Flash single firmware with esptool
```sh
esptool.py --chip=esp32 -p /dev/ttyACM0 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x0 firmware.bin
```


### Serial monitor
```sh
idf.py -p /dev/ttyACM0 monitor
```