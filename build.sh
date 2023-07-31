# use: ./build.sh <version>
echo "Run build version: " $1
idf.py build -D PROJECT_VER=$1
echo "Run merge firmware"
echo "Binary files"
ls -la build/bootloader | grep \.bin$
ls -la build/partition_table | grep \.bin$
ls -la build | grep \.bin$
esptool.py --chip esp32 merge_bin -o firmware.bin --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0xd000 build/ota_data_initial.bin  0x10000 build/ground-station.bin
echo "Output file"
ls -la | grep \.bin$
