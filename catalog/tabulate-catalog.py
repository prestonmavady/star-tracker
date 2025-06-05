# /star-tracker     rev. 06022025       preston mavady
# ----------------------------------------------------------
# Takes in a star catalog, outputs a binned version with
# a bin resolution of 0.1 degrees (as an EEPROM flash-ready
# bytestream)

# Pseudocode:

# 1. CONVERT CATALOG TO UNIT VECTORS: 
# catalog is stored as ID, RA, DEC, Vmag in a 4 column csv
# generate unit vectors (ignore Vmag)
    # append ID  |  X  |  Y  | Z  to a 4 column csv

# 2. GENERATE A LIST OF ALL PAIRWISE ANGLES:
# for nth star id
    # for all other stars
    # how to avoid double-counting?
        # calculate the angle between them
        # ignore lines that are empty/wrong format
            # append ID1  |  ID2  | angle to a 3-column csv

# 3. CREATE A BINNED LIST OF PAIRWISE ANGLES (BIN RESOLUTION 0.1deg):
# we are using 840 bins (0–84° range with 0.1° resolution)
# any angles greater than 84 will be ignored
# for each bin
    # collect [index1, index2] pairs
    # store only the pair; bin index implicitly represents angle
    # each pair is 2 bytes (1 byte per index)

# 4. GENERATE NEEDED LOOK-UP-TABLES:
# LUT 1) INDX : HIP IDs
    # sort HIP IDs
    # store a mapping from HIP ID → small integer index (0–255)
    # output index-to-HIP LUT (index-to-hip.csv)
# LUT 2) INDX : Bin Addresses

# 5. GENERATE BYTESTREAM
# turn the binned data into a byte stream for EEPROM flash:
    # for each bin:
        # 1 byte: number of pairs in bin
        # 2 bytes per pair: [ID1, ID2]

# print out how many bytes long the bytestream is.
# ----------------------------------------------------------

import numpy as np
import csv
import os

c_code_output_dir = "c_code"
os.makedirs(c_code_output_dir, exist_ok=True)

# Output file names
catalog_file = '1_catalog_ra_dec.csv'
angle_file = '2_pairwise_angles.csv'
unit_vector_file = '3_catalog_x_y_z.csv'
binned_file = '4_binned_catalog.csv'
memory_map_file = "5_eeprom_memory_map.txt"

# Defines for binning section
BIN_COUNT = 840
BIN_WIDTH = 0.1
PAIR_SIZE = 2

# Defines for memory-mapping section
EEPROM_ADDR = "0x50"
EEPROM_LINE_WIDTH = 8

print(f"\n >->-* STARTING CATALOG TABULATION. *-<-<.")

# --------------------------------------------------------
# STEP 1: Convert Catalog to Unit Vectors
# --------------------------------------------------------
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

if len(hip_to_index) > 256:
    raise ValueError("Catalog exceeds 1-byte index limit (256 stars).")

with open(unit_vector_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['HIP', 'X', 'Y', 'Z'])
    for hip, vec in unit_vectors:
        writer.writerow([hip] + vec.tolist())

print(f"\n    ...Parsed {len(unit_vectors)} stars from catalog.")

# --------------------------------------------------------
# STEP 2: Generate a List of All Pairwise Angles
# --------------------------------------------------------
angle_data = []
for i in range(len(unit_vectors)):
    hip1, vec1 = unit_vectors[i]
    for j in range(i + 1, len(unit_vectors)):
        hip2, vec2 = unit_vectors[j]
        dot = np.clip(np.dot(vec1, vec2), -1.0, 1.0)
        angle_deg = np.degrees(np.arccos(dot))
        if angle_deg <= BIN_COUNT * BIN_WIDTH:
            angle_data.append((hip1, hip2, angle_deg))

# ── WRITE OUT A CSV FOR ALL PAIRWISE ANGLES 
with open(angle_file, 'w', newline='') as csvf:
    writer = csv.writer(csvf)
    writer.writerow(['ID1', 'ID2', 'Angle_deg'])
    for id1, id2, angle in angle_data:
        writer.writerow([id1, id2, f"{angle:.6f}"])
print(f"\n    ...Wrote {len(angle_data)} pairwise angles to '{angle_file}'.")

# Track discarded angles
total_pairs_possible = len(unit_vectors) * (len(unit_vectors) - 1) // 2
discarded_pairs = total_pairs_possible - len(angle_data)

print(f"\n    ...Generated {total_pairs_possible} pairwise angles.")
print(f"    ...Discarded {discarded_pairs} pairwise angles (>84°).")
print(f"    ...Kept {len(angle_data)} pairwise angles.")

# --------------------------------------------------------
# STEP 3: Create a Binned List of All Pairwise Angles (0.1deg)
# --------------------------------------------------------
binned = [[] for _ in range(BIN_COUNT)]
for hip1, hip2, angle in angle_data:
    bin_index = int(angle // BIN_WIDTH)
    if 0 <= bin_index < BIN_COUNT:
        idx1 = hip_to_index[hip1]
        idx2 = hip_to_index[hip2]
        binned[bin_index].append((idx1, idx2))

# Create '4_binned_catalog.csv' file
address_counter = 0
with open(binned_file, 'w', newline='') as f:
    writer = csv.writer(f)
    for bin_index, contents in enumerate(binned):
        writer.writerow([f'BIN{bin_index}', len(contents), address_counter])
        address_counter += 1 + len(contents) * PAIR_SIZE
        for idx1, idx2 in contents:
            writer.writerow([idx1, idx2])
        writer.writerow([])
        writer.writerow([])

# --------------------------------------------------------
# STEP 4: Generate EEPROM Bytestream
# --------------------------------------------------------

bytestream = []
bin_angle_to_address_lut = []
address_counter = 0

for bin_index in range(BIN_COUNT):
    contents = binned[bin_index]
    num_pairs = len(contents)

    bin_angle_to_address_lut.append(address_counter)             # Record address of this bin
    bytestream.append(num_pairs & 0xFF)                 # 1 byte: number of pairs
    for idx1, idx2 in contents:
        bytestream.append(idx1 & 0xFF)
        bytestream.append(idx2 & 0xFF)

    address_counter += 1 + num_pairs * 2               # Update byte position

# --------------------------------------------------------
# STEP 5: Generate Needed Look-Up Tables
# --------------------------------------------------------

# 1. LUT: bin_index : byte address
# Done! already built during bytestream generation

# 2. LUT: index : HIP
index_to_hip_lut = [None] * len(hip_to_index)
for hip, idx in hip_to_index.items():
    index_to_hip_lut[idx] = hip

# --------------------------------------------------------
# STEP 6: Generate Useable .c/.h Files for STM32
# --------------------------------------------------------

def float_to_byte_unit_vector(val):
    """Map float in [-1, 1] → uint8_t in [0, 255]"""
    val = max(-1.0, min(1.0, val))
    return int((val + 1.0) * 127.5 + 0.5)

catalog_bytes = []
index_to_hip_lut = []

for idx, (hip, vec) in enumerate(unit_vectors):
    x_byte = float_to_byte_unit_vector(vec[0])
    y_byte = float_to_byte_unit_vector(vec[1])
    z_byte = float_to_byte_unit_vector(vec[2])

    catalog_bytes.append(idx & 0xFF)  # 1-byte HIP index
    catalog_bytes.append(x_byte)
    catalog_bytes.append(y_byte)
    catalog_bytes.append(z_byte)

    index_to_hip_lut.append(hip)

# ---------- Write catalog_luts.h ----------
with open(os.path.join(c_code_output_dir, "catalog_luts.h"), 'w') as f:
    f.write("#ifndef CATALOG_LUTS_H\n#define CATALOG_LUTS_H\n\n")
    f.write("#include <stdint.h>\n\n")
    f.write("extern const uint8_t catalog[];\n")
    f.write("extern const uint16_t index_to_hip_ID_lut[];\n")
    f.write("extern const uint16_t bin_angle_to_address_lut[];\n")
    f.write("extern const size_t index_count;\n")
    f.write("extern const size_t bin_count;\n\n")
    f.write("#endif // CATALOG_LUTS_H\n")

# ---------- Write catalog_luts.c ----------
with open(os.path.join(c_code_output_dir, "catalog_luts.c"), 'w') as f:
    f.write("// catalog_luts.c - LUT data for star tracker catalog\n\n")
    f.write("#include <stdint.h>\n\n")

    # catalog[]
    f.write("const uint8_t catalog[] = {\n")
    f.write("    //Catalog stored as:\n")
    f.write("    //[HIP_ID],[x],[y],[z] (1 byte each)\n")
    for i in range(0, len(catalog_bytes), 4):
        entry = catalog_bytes[i:i+4]
        hip_index = entry[0]
        x, y, z = entry[1], entry[2], entry[3]
        f.write(f"    0x{hip_index:02X}, 0x{x:02X}, 0x{y:02X}, 0x{z:02X},\n")
    f.write("};\n\n")

    # index_to_hip_ID_lut[]
    f.write("const uint16_t index_to_hip_ID_lut[] = {\n    ")
    for i, hip in enumerate(index_to_hip_lut):
        f.write(f"0x{hip:04X}")
        if i != len(index_to_hip_lut) - 1:
            f.write(", ")
        if (i + 1) % 8 == 0:
            f.write("\n    ")
    f.write("\n};\n")
    f.write(f"const size_t index_count = sizeof(index_to_hip_ID_lut) / sizeof(index_to_hip_ID_lut[0]);\n\n")

    # bin_angle_to_address_lut[]
    f.write("const uint16_t bin_angle_to_address_lut[] = {\n    ")
    for i, addr in enumerate(bin_angle_to_address_lut):
        f.write(f"0x{addr:04X}")
        if i != len(bin_angle_to_address_lut) - 1:
            f.write(", ")
        if (i + 1) % 8 == 0:
            f.write("\n    ")
    f.write("\n};\n")
    f.write(f"const size_t bin_count = sizeof(bin_angle_to_address_lut) / sizeof(bin_angle_to_address_lut[0]);\n")

# 2. Write catalog_bytestream.h/.c
with open(os.path.join(c_code_output_dir, "catalog_bytestream.h"), 'w') as f:
    f.write("#ifndef CATALOG_BYTESTREAM_H\n#define CATALOG_BYTESTREAM_H\n\n")
    f.write("#include <stdint.h>\n\n")
    f.write("extern const uint8_t catalog_bytestream[];\n")
    f.write("extern const size_t bytestream_len;\n\n")
    f.write("#endif // CATALOG_BYTESTREAM_H\n")

with open(os.path.join(c_code_output_dir, "catalog_bytestream.c"), 'w') as f:
    f.write("// catalog_bytestream.c - EEPROM data array for flashing\n\n")
    f.write('#include <stdint.h>\n\n')

    f.write("const uint8_t catalog_bytestream[] = {\n    ")
    for i, byte in enumerate(bytestream):
        f.write(f"0x{byte:02X}")
        if i != len(bytestream) - 1:
            f.write(", ")
        if (i + 1) % 16 == 0:
            f.write("\n    ")
    f.write("\n};\n")
    f.write(f"const size_t bytestream_len = sizeof(catalog_bytestream);\n")

# --------------------------------------------------------
# STEP 7: Output Visual EEPROM Memory Map (as a text file)
# --------------------------------------------------------

# EEPROM1: [HIP][RA][DEC] — derived from unit_vectors
eeprom1_data = []
for i, (hip, vec) in enumerate(unit_vectors):
    hip_id = hip & 0xFF
    ra = (i * 2) % 256  # dummy RA
    dec = (i * 3) % 256  # dummy Dec
    eeprom1_data.extend([hip_id, ra, dec])

# Box drawing function
def format_block(title, i2c_addr, data_bytes, width=EEPROM_LINE_WIDTH):
    lines = []
    border = "─" * 66
    lines.append(f"┌─ {title} | Addr: {i2c_addr} " + "─" * (66 - len(title) - len(i2c_addr) - 15) + "┐")
    lines.append("│ Addr   | Data (Hex)                                             │")
    lines.append("├────────┼───────────────────────────────────────────────────────┤")
    for addr in range(0, len(data_bytes), width):
        chunk = data_bytes[addr:addr+width]
        hex_data = " ".join(f"{b:02X}" for b in chunk)
        lines.append(f"│ {addr:06X} | {hex_data:<53} │")
    lines.append("└" + border + "┘")
    return "\n".join(lines)

# Combine blocks
map_text = "\n\n".join([
    format_block("Onboard STM32 Flash (1 MB: Star Catalog)", "N/A", eeprom1_data),
    format_block("EEPROM2: Angle Bin Data", EEPROM_ADDR, bytestream),
])

with open(memory_map_file, "w") as f:
    f.write(map_text)


# --------------------------------------------------------
# Final Terminal Output Detailing Completion:
# --------------------------------------------------------

print("\nPipeline complete.")

total_pairs = 0
for i, contents in enumerate(binned):
    total_pairs += len(contents)
pair_data_bytes = total_pairs * 2   # 2 bytes per pair (idx1 + idx2)
bin_header_bytes = BIN_COUNT        # 1 byte per bin for number of pairs
total_bytes = len(bytestream)

print("\nBytestream Summary:")
print(f"  {BIN_COUNT} bins * 1 byte             = {bin_header_bytes} bytes (bin headers)")
print(f"  {total_pairs} pairs * 2 bytes         = {pair_data_bytes} bytes (pair data)")
print(f"  Total EEPROM bytestream size  = {total_bytes} bytes")

