# /star-tracker     rev. 05312025       preston mavady
# ----------------------------------------------------------
# Centroiding Algorithm
#   Optimized for pixel math - high speed and accuracy.
#   > Contrast Enhancement (brights get brighter,
#     darks get darker)
#   > Gaussian Blur To Suppress Noise (random noisy
#     pixels will get reduced by surrounding dark)
#   > Adaptive Percentile-based Thresholding
#     to avoid light pollution/noise.
#   > Accepts/Rejects Pixels based on Threshold
#   > Differentiates Stars from Noise and Objects
#     by checking pixel cluster size.

import cv2
import numpy as np
from scipy.ndimage import label


def find_centroids(image, min_area=3, max_area=200):
    # 0. Convert the image to greyscale
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    # 1. Local contrast enhancement (adaptive histogram equalization)
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
    image = clahe.apply(image)

    # 2. Slight Gaussian Blur
    image = cv2.GaussianBlur(image, (3, 3), 0)

    # 3. Adaptive Thresholding
    percentile = 99  # pick top 1% brightest pixels
    # create an adaptive 'brightness' threshold (0-255) from image
    threshold = np.percentile(image, percentile)
    print(f"Using adaptive threshold: {threshold:.2f}")
    # create a binary image based on threshold (sets pixels to either 0 or 255)
    _, binary = cv2.threshold(image, threshold, 255, cv2.THRESH_BINARY)

    # 4. Cluster Detection
    # define the cluster structure as a 3x3 pixel matrix
    # > this means a pixel will be checked in all 8 possible directions
    structure = np.ones((3, 3), dtype=np.uint8)
    # find all the clusters in the binary image
    labeled, num_features = label(binary, structure=structure)
    # iterate through each cluster, reject if area is too large, append centroids to list
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
