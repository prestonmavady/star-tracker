# Functions Needed:
# Camera config?
# Capture Frame



# Save Frame
# so it seems like its going to need to save it somewhere in memory (by setting a path)
image_path = os.path.join(image_folder, name + ".jpg")
# and then load the image into a ndarray (image) variable
image_raw = cv2.imread(image_path)
# we can then call the centroids function on the image in main
centroids = find_centroids(image_raw, threshold=230)