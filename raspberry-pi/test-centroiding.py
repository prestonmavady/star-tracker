import os
import cv2
import numpy as np
from scipy.ndimage import label
import centroiding

# === Setup your folders ===
input_folder = "/Users/preston/Desktop/centroiding/STARFIELDS"
output_folder = "/Users/preston/Desktop/centroiding/OUTPUTS"
os.makedirs(output_folder, exist_ok=True)

# === Prompt user for input/output names ===
input_name = input("Enter input file name (e.g., sf1): ").strip()
output_name = input("Enter desired output file name (e.g., opt1): ").strip()

input_path = os.path.join(input_folder, input_name + ".jpg")
output_path = os.path.join(output_folder, output_name + ".jpg")

# === Load and process image ===
img_raw = cv2.imread(input_path)
if img_raw is None:
    print(f"Error: Could not load image '{input_path}'")
    exit()

centroids = find_centroids(gray, threshold=200)

# === Print star locations ===
print(f"\n Found {len(centroids)} stars in '{input_name}.jpg':")
for i, (x, y) in enumerate(centroids):
    print(f"  Star {i+1}: x = {x:.2f}, y = {y:.2f}")

# === Annotate and save output image ===
annotated = img_raw.copy()
for (x, y) in centroids:
    cv2.circle(annotated, (int(x), int(y)), radius=2, color=(0, 0, 255), thickness=-1)

cv2.imwrite(output_path, annotated)
print(f"\n Saved annotated image as '{output_path}'")
