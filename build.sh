echo "Run build commands"
idf.py build -D PROJECT_VER="${{ env.GitVersion_SemVer }}"
pip install esptool
cd ./build
ls -d
python -m esptool --chip esp32 merge_bin -o firmware.bin --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 bootloader/bootloader.bin 0x10000 ground-station.bin 0x8000 partition_table/partition-table.bin 0xd000 ota_data_initial.bin