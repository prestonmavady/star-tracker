import os
import cv2
import numpy as np
from scipy.ndimage import label

def find_centroids(image, threshold=255, min_area=3, max_area=200):
    # 1. Local contrast enhancement (adaptive histogram equalization)
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
    image = clahe.apply(image)

    # 2. Slight blur (optional)
    image = cv2.GaussianBlur(image, (3, 3), 0)

    # 3. Threshold
    percentile = 99  # pick top 2% brightest pixels
    threshold = np.percentile(image, percentile)
    print(f"Using adaptive threshold: {threshold:.2f}")

    _, binary = cv2.threshold(image, threshold, 255, cv2.THRESH_BINARY)

    # 4. Show binary image for debug
    cv2.imshow("Thresholded Binary", binary)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

    # 5. Connected components
    structure = np.ones((3, 3), dtype=np.uint8)
    labeled, num_features = label(binary, structure=structure)
    centroids = []

    print(f"Found {num_features} blobs")
    for i in range(1, num_features + 1):
        mask = labeled == i
        area = np.sum(mask)
        print(f"  Blob {i}: area={area}, accepted={min_area <= area <= max_area}")
        if min_area <= area <= max_area:
            weights = image * mask
            total_intensity = np.sum(weights)
            if total_intensity > 0:
                y_idx, x_idx = np.indices(image.shape)
                x_center = np.sum(x_idx * weights) / total_intensity
                y_center = np.sum(y_idx * weights) / total_intensity
                centroids.append((x_center, y_center))
    return centroids


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

# convert image to grayscale
gray = cv2.cvtColor(img_raw, cv2.COLOR_BGR2GRAY)

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
