import spidev
import struct
import time

class SPIDevice:
    def __init__(self, bus=0, device=0, max_speed_hz=1000000, mode=0b00):
        self.spi = spidev.SpiDev()
        self.spi.open(bus, device)
        self.spi.max_speed_hz = max_speed_hz
        self.spi.mode = mode
        self.spi.bits_per_word = 8
        print(f"SPI opened on bus {bus}, device {device}")

    def send_centroids(self, centroids):
        """
        centroids: List of (x, y) float tuples
        """
        buffer = b''.join(struct.pack('<ff', x, y) for x, y in centroids)
        print(f"Sending {len(centroids)} centroids ({len(buffer)} bytes)")
        
        # Send in chunks to avoid buffer overflows
        chunk_size = 32  # Adjust based on STM32 buffer
        for i in range(0, len(buffer), chunk_size):
            chunk = buffer[i:i+chunk_size]
            self.spi.xfer2(list(chunk))
            time.sleep(0.001)  # short delay to avoid overrunning STM32

    def close(self):
        self.spi.close()
        print("SPI connection closed")

# Example usage
if __name__ == "__main__":
    centroids = [
        (199.88, 13.13),
        (436.83, 13.13),
        (289.40, 17.18),
        (526.03, 17.03),
        (52.50, 17.40),
        (162.70, 17.30),
        (398.03, 22.26),
        (453.50, 21.50)
    ]

    spi_device = SPIDevice()
    try:
        spi_device.send_centroids(centroids)
    finally:
        spi_device.close()
