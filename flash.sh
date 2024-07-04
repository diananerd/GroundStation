# use: ./flash.sh <port>
esptool.py --chip=esp32 -p $1 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x0 firmware.bin