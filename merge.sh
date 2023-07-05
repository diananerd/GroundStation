echo "Run merge commands"
sudo pip install esptool
cd ./build
pwd
ls -a
sudo python -m esptool --chip esp32 merge_bin -o firmware.bin --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 bootloader/bootloader.bin 0x10000 ground-station.bin 0x8000 partition_table/partition-table.bin
