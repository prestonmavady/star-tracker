# Raspberry Pi

The Pi boots and runs one continuous loop. Since the Pi emulates an FPGA SPI Controller, we structure our code as a deterministic pipeline.

Boot > 
  LIS Mode > Capture Frame > Greyscale and Preprocess > Extract Centroids > Format SPI-Ready Binary Packet > SPI TX
  Tracking Mode > 


### SPI Protocol
> Our Pi acts as the SPI Controller, and is setup for simplex @ 4MHz
