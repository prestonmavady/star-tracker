# to do:
# add headers
# Capture Frame Function
# Package SPI-Ready Binary Packet Function
# SPI TX Setup

import cv2
import numpy as np
from scipy.ndimage import label

##
# args:     image
# returns:  list of float tuples
##
def find_centroids(image, threshold=255, min_area=3, max_area=200):
    # 0. Convert the image to greyscale
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
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
