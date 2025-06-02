# /star-tracker     rev. 06022025       preston mavady
# ----------------------------------------------------------
# Takes in a star catalog, outputs pairwise angles.
# Pseudocode:

# 1. GENERATE UNIT VECTORS: (catalog-as-unit-vectors.csv)
# catalog is stored as ID, RA, DEC, Vmag in a 4 column csv
# generate unit vectors (ignore Vmag)
    # append ID  |  X  |  Y  | Z  to a 4 column csv

# 2. MAP HIP IDs TO INDICES
# sort HIP IDs
# store a mapping from HIP ID → small integer index (0–255)
# output index-to-HIP LUT (index-to-hip.csv)

# 3. GENERATE PAIRWISE ANGLES: (pairwise-angles.csv)
# for nth star id
    # for all other stars
    # how to avoid double-counting?
        # calculate the angle between them
        # ignore lines that are empty/wrong format
            # append ID1  |  ID2  | angle to a 3-column csv

# 4. BIN PAIRWISE ANGLES AT 0.1 DEGREE RESOLUTION (binned-catalog.csv)
# we are using 840 bins (0–84° range with 0.1° resolution)
# any angles greater than 84 will be ignored
# for each bin
    # collect [index1, index2] pairs
    # store only the pair; bin index implicitly represents angle
    # each pair is 2 bytes (1 byte per index)

# 5. GENERATE LOOK-UP-TABLE:
# output bin → address mapping into a CSV (one byte per bin label)

# 6. GENERATE BITSTREAM (to flash EEPROM)
# turn the binned data into a byte stream:
    # for each bin:
        # 1 byte: bin label
        # 2 bytes per pair: [index1 (1B), index2 (1B)]
# ----------------------------------------------------------

import numpy as np
import csv

catalog_file = 'catalog.csv'
unit_vector_file = 'catalog-as-unit-vectors.csv'
angle_file = 'pairwise-angles.csv'
binned_file = 'binned-catalog.csv'
lut_file = 'lookup-table.csv'
bitstream_file = 'bitstream.txt'
index_to_hip_file = 'index-to-hip.csv'

BIN_COUNT = 840
BIN_WIDTH = 0.1
PAIR_SIZE = 2  # 1 byte index1 + 1 byte index2

# ----------------------------
# STEP 1: Unit Vector Conversion
# ----------------------------
def radec_to_unit_vector(ra_deg, dec_deg):
    ra = np.radians(ra_deg)
    dec = np.radians(dec_deg)
    x = np.cos(dec) * np.cos(ra)
    y = np.cos(dec) * np.sin(ra)
    z = np.sin(dec)
    return np.array([x, y, z])

unit_vectors = []
with open(catalog_file, 'r') as f:
    reader = csv.reader(f)
    next(reader)
    for row in reader:
        if len(row) < 4:
            continue
        hip_str, ra_str, dec_str, _ = row
        try:
            hip = int(hip_str)
            ra = float(ra_str)
            dec = float(dec_str)
            vec = radec_to_unit_vector(ra, dec)
            unit_vectors.append((hip, vec))
        except ValueError:
            continue

unit_vectors.sort(key=lambda x: x[0])  # sort by HIP ID
hip_to_index = {hip: idx for idx, (hip, _) in enumerate(unit_vectors)}

if len(hip_to_index) > 257:
    raise ValueError("Catalog exceeds 1-byte index limit (255 stars).")

with open(index_to_hip_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['Index', 'HIP'])
    for hip, idx in hip_to_index.items():
        writer.writerow([idx, hip])

with open(unit_vector_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['HIP', 'X', 'Y', 'Z'])
    for hip, vec in unit_vectors:
        writer.writerow([hip] + vec.tolist())

print(f"Parsed {len(unit_vectors)} stars from catalog.")

# ----------------------------
# STEP 2: Pairwise Angles
# ----------------------------
angle_data = []
for i in range(len(unit_vectors)):
    hip1, vec1 = unit_vectors[i]
    for j in range(i + 1, len(unit_vectors)):
        hip2, vec2 = unit_vectors[j]
        dot = np.clip(np.dot(vec1, vec2), -1.0, 1.0)
        angle_deg = np.degrees(np.arccos(dot))
        if angle_deg <= BIN_COUNT * BIN_WIDTH:
            angle_data.append((hip1, hip2, angle_deg))

with open(angle_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['HIP1', 'HIP2', 'angle_deg'])
    writer.writerows(angle_data)

print(f"Computed {len(angle_data)} valid pairwise angles.")

# ----------------------------
# STEP 3: Bin Angle Data (0.1°)
# ----------------------------
binned = [[] for _ in range(BIN_COUNT)]
for hip1, hip2, angle in angle_data:
    bin_index = int(angle // BIN_WIDTH)
    if 0 <= bin_index < BIN_COUNT:
        idx1 = hip_to_index[hip1]
        idx2 = hip_to_index[hip2]
        binned[bin_index].append((idx1, idx2))

address_counter = 0
lookup_table = []

with open(binned_file, 'w', newline='') as f:
    writer = csv.writer(f)
    for bin_index, contents in enumerate(binned):
        writer.writerow([f'BIN{bin_index}', len(contents), address_counter])
        lookup_table.append(address_counter)
        address_counter += 1 + len(contents) * PAIR_SIZE
        for idx1, idx2 in contents:
            writer.writerow([idx1, idx2])
        writer.writerow([])
        writer.writerow([])
        writer.writerow([])

# ----------------------------
# STEP 4: Generate Lookup Table
# ----------------------------
with open(lut_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['Bin', 'Start Address'])
    for i, addr in enumerate(lookup_table):
        writer.writerow([i, addr])

# ----------------------------
# STEP 5: Generate EEPROM Bitstream
# ----------------------------
bitstream = []
for bin_index, contents in enumerate(binned):
    bitstream.append(bin_index)
    for idx1, idx2 in contents:
        bitstream.append(idx1 & 0xFF)
        bitstream.append(idx2 & 0xFF)

with open(bitstream_file, 'w') as f:
    for byte in bitstream:
        f.write(f"{byte}\n")

print("Pipeline complete:")
print(f"  Unit vectors saved to: {unit_vector_file}")
print(f"  Pairwise angles saved to: {angle_file}")
print(f"  Binned catalog saved to: {binned_file}")
print(f"  Lookup table saved to: {lut_file}")
print(f"  Bitstream saved to: {bitstream_file}")
print(f"  Index-to-HIP LUT saved to: {index_to_hip_file}")
print("\nBin Summary:")
for i, contents in enumerate(binned):
    print(f"  BIN{i:03}: {len(contents)} pairs")
