# main
1. handles inbound spi rx
2. checks if eeprom has been flashed (checks first 8 bytes)
3. runs gvalg when spi rx has finished
4. stores gvalg'd IDs
5. passes gvalg'd IDs + catalog to quest
6. runs quest
7. handles outbound uart tx


### files needed:

main.c

spi.c
 - SPI_init
 - SPI_rx
eeprom.c
 - eeprom_init
 - eeprom_flash
 - eeprom_check
 - eeprom_write
 - eeprom_read
 - eeprom_flash
 - eeprom_toggle_wp

gvalg.c

quest.c

lpuart.c
 - LPUART_init
 - LPUART_tx


### helper files:

delay.c