import os
import numpy as np
import argparse
import cv2
import requests 



images = {}

def imagetohis(url):
    # d,filename = os.path.split(imagePath)
    # #filename = imagePath[imagePath.rfind("/") + 1:]
    # image = cv2.imread(imagePath)
    # #images[filename] = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    # hist = cv2.calcHist([image], [0, 1, 2], None, [8, 8, 8],[0, 256, 0, 256, 0, 256])
    # hist = cv2.normalize(hist, hist).flatten()
    
    resp = requests.get(url, stream=True).raw
    image = np.asarray(bytearray(resp.read()), dtype="uint8")

    image = cv2.imdecode(image, cv2.IMREAD_COLOR)
    hist = cv2.calcHist([image], [0, 1, 2], None, [8, 8, 8],[0, 256, 0, 256, 0, 256])
    hist = cv2.normalize(hist, hist).flatten()

    return hist
    

    
    
    