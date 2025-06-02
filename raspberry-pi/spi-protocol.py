# /star-tracker     rev. 05312025       preston mavady
# ----------------------------------------------------------
# Tailored SPI Interface Implementation
#   'SPIDevice' class encapsulates state and behavior
#   > Fixed-point encoding with scaling (default: x1000)
#   > Start (0xAA) and Stop (0x55) byte framing
#   > Chunked transfer (32-byte blocks for STM32 SPI buffer)

import spidev
import struct
import time

class SPIDevice:
    def __init__(self, bus=0, device=0, max_speed_hz=4000000, mode=0b00):
        # Create a spidev instance, initialize our SPI hardware parameters
        self.spi = spidev.SpiDev()
        self.spi.open(bus, device)
        self.spi.max_speed_hz = max_speed_hz
        self.spi.mode = mode
        self.spi.bits_per_word = 8
        print(f"SPI opened on bus {bus}, device {device}")
              
    def send_centroids(self, centroids, scale=1000):
        # ------ BUILD PACKET ------
        START_BYTE = 0b10101010
        STOP_BYTE  = 0b01010101
        # create a new binary buffer, start by appending start byte
        buffer = bytes(START_BYTE)
        # take the float tuples and convert to uint32's with scale of 1000, append to buffer
        """ NOTE: when STM32 recieves, make sure to divide by 1000 """
        for x, y in centroids:
            fx = int(round(x * scale))
            fy = int(round(y * scale))
            buffer += struct.pack('<II', fx, fy)  # <II is two uint32_t integers (8 bytes)
        # append STOP byte
        buffer += bytes(STOP_BYTE)
        print(f"Buffer contains {len(centroids)} centroids as ({(len(buffer)-2)} bytes) (x1000 scale)")

        # ------ SEND PACKET ------
        # Send in chunks to avoid buffer overflows
        chunk_size = 32  # STM32 has 32-byte buffers
        for i in range(0, len(buffer), chunk_size):
            chunk = buffer[i:i+chunk_size]
            self.spi.xfer2(list(chunk))
            time.sleep(0.001)  # short delay to avoid overrunning STM32
        print("SPI Tx complete.")

    def close(self):
        self.spi.close()
        print("SPI connection closed")