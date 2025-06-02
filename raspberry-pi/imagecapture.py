# /star-tracker     rev. 05312025       preston mavady
# ----------------------------------------------------------
# Raspberry Pi Image Capture
# @ Target Hardware: RPi Zero 2 W + Waveshare IMX290-83
# ! Make sure you install the IMX290 overlay from waveshare.
#   > Self-Aware Incremental/Time-Stamping File Naming.
#   > Outputs .jpg to home/pi/images folder.
#   > Total time to take an image:

from picamera2 import Picamera2
import os
import time
import re
from datetime import datetime
import cv2

# ------ DEFINES ------
FRAME_WIDTH = 1920
FRAME_HEIGHT = 1080
IMAGE_FOLDER = "/home/pi/images"
BASE_NAME = "starfield"
EXTENSION = ".jpg"

# ------ MAKE SURE OUTPUT FOLDER EXISTS ------
os.makedirs(IMAGE_FOLDER, exist_ok=True)

# ------ FILE NAMING (TIMESTAMP + INCREMENT) ------
def get_incremented_timestamped_name(folder, base_name=BASE_NAME, extension=EXTENSION):
    existing = os.listdir(folder)
    pattern = re.compile(rf"{re.escape(base_name)}_(\d+)_\d{{8}}_\d{{6}}{re.escape(extension)}")
    numbers = [int(m.group(1)) for f in existing if (m := pattern.match(f))]
    next_num = max(numbers, default=0) + 1
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    return f"{base_name}_{next_num:03d}_{timestamp}{extension}"

# ------ CAMERA MANAGER CLASS ------
class CameraManager:
    def __init__(self, resolution=(FRAME_WIDTH, FRAME_HEIGHT)):
        self.picam2 = Picamera2()
        config = self.picam2.create_still_configuration(main={"size": resolution})
        self.picam2.configure(config)
        self.picam2.start()
        time.sleep(1)  # One-time warm-up
        print("[CameraManager] Camera initialized and running.")

    def capture_image(self, save_path):
        try:
            frame = self.picam2.capture_array()
            cv2.imwrite(save_path, frame)
            print(f"[Camera] Image saved to {save_path}")
            return True
        except Exception as e:
            print(f"[Camera] Capture error: {e}")
            return False

    def close(self):
        self.picam2.close()
        print("[CameraManager] Camera closed.")

# === Main ===
if __name__ == "__main__":
    camera = CameraManager()
    image_name = get_incremented_timestamped_name(IMAGE_FOLDER)
    image_path = os.path.join(IMAGE_FOLDER, image_name)

    success = camera.capture_image(image_path)
    camera.close()

    if not success:
        exit(1)
