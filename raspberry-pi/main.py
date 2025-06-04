# /star-tracker     rev. 05312025       preston mavady
# ----------------------------------------------------------
# Master control for star tracker system
#   > Captures image
#   > Extracts star centroids
#   > Sends data over SPI to STM32
#   > Logs output

import os
import cv2

from imagecapture import CameraManager, get_incremented_timestamped_name, IMAGE_FOLDER
from centroiding import find_centroids
from spi-protocol import SPIDevice

def main():
    # ----- Step 1: Capture image -----
    camera = CameraManager()
    image_name = get_incremented_timestamped_name(IMAGE_FOLDER)
    image_path = os.path.join(IMAGE_FOLDER, image_name)

    if not camera.capture_image(image_path):
        print("[Main] Image capture failed. Exiting.")
        camera.close()
        return
    camera.close()

    # ----- Step 2: Load and process image -----
    print(f"[Main] Loading image: {image_path}")
    image = cv2.imread(image_path)
    if image is None:
        print("[Main] Failed to load image.")
        return

    centroids = find_centroids(image)
    print(f"[Main] Found {len(centroids)} centroids.")

    # ----- Step 3: Send via SPI -----
    spi = SPIDevice()
    try:
        spi.send_centroids(centroids)
    finally:
        spi.close()

    print("[Main] Complete.")

if __name__ == "__main__":
    main()
