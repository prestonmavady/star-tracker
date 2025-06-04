# Star Catalog

### Tabulate-Catalog Function:
'tabulate-catalog.py' takes in a csv-formatted star catalog and outputs all the needed files to interface with our geometric voting algorithm.

We hope you find the code to be commented in detail, and well-outlined in the provided pseudocode.

### Included (Generated) Files:

```
catalog.csv
> contains the 256 brightest stars in the Hipparcos catalog, stored as [HIP_ID, RAdeg, DEdeg, and Vmag]


catalog-as-unit-vectors.csv
> contains the 256 brightest stars in the Hipparcos catalog, stored as [HIP_ID, x, y, z]


pairwise-angles.csv
> all combinations of star pairs and their calculated vector angle, stored as [ID1, ID2, angle]


binned-catalog.csv
> all star pairs sorted by angle into bins of 0.1 degree angles, stored as: 
Bin Label Entry: [Bin#, NumPairs, Addr] and 
Pair Entry: [ID1, ID2]


lookup-table.csv
> a lookup-table mapping pairwise angle to bin address in memory, stored as [bin#, addr]
e.x. to find the starting address for bin 36.8 degrees, you would index into the LUT table(368-1) and get the output '7967' corresponding to that address on the EEPROM.


index-to-hip.csv
> a lookup-table mapping a star's HIP ID to a number between 0-255 so we can store HIP ID's (which go up to 218,000) in an 8-bit format.