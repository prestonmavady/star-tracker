# Raspberry Pi

The need for a Raspberry Pi comes from our Sony IMX290 STARVIS camera module, which only operates with the Pi's CSI camera interface.
Using a Pi for this implementation _is admittedly overkill_. Since the Pi emulates an FPGA SPI Controller, we restrict ourselves to these rules:
```
   starting the loop on boot (using systemd),
   structured as a deterministic pipeline (state machine),
   only using fixed-point math (no floats)
```

**Implementation:** Boot > Capture Frame > Greyscale and Preprocess > Extract Centroids > Format SPI-Ready Binary Packet > SPI TX

### SPI Protocol
> Our Pi acts as the SPI Controller (simplex @ 4MHz) through Python's 'spidev' library.

##### Packet Formatting:
```
Byte   Meaning
----   --------
0xAA   Start byte
1      Star count (uint32)
2–5    X1 coord   (uint32)
6–9    Y1 coord   (uint32)
...    more stars
407    0x55 (end byte)
```

##### Transmission Rate:
We're sending the centroids of the top 100 stars in our image. Each centroid is composed of 8 bytes _(2x 32-bit Fixed-Point Coordinates)_. At 4 MHz that's:
> 6464 bits / 4,000,000 bits/sec = **1.6ms**