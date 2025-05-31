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
> Our Pi acts as the SPI Controller (simplex @ 4MHz)

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
### To-Do
Capture Frame Function, Package SPI-Ready Binary Packet Function, SPI TX Setup
