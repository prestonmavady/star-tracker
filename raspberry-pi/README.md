# Raspberry Pi

Using an Pi for this implementation is overkill. Since the Pi emulates an FPGA SPI Controller, we restrict ourselves to these rules:
```
   starting the loop on boot (using systemd),
   structured as a deterministic pipeline (state machine),
   only using fixed-point math (no floats)
```

Boot > Capture Frame (not done) > Greyscale and Preprocess > Extract Centroids > Format SPI-Ready Binary Packet (not done) > SPI TX (not done)

### SPI Protocol
> Our Pi acts as the SPI Controller, and is setup for simplex @ 4MHz

##### Packet Formatting:
```
Byte   Meaning
----   --------
0xAA   Start byte
1      Star count (uint16)
2–3    X1 coord (uint16)
4–5    Y1 coord (uint16)
...    more stars
N      0x55 (end byte)
```
