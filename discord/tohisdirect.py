import os
import numpy as np
import argparse
import cv2
import requests 
import glob


images = {}
def tohis (name):
    for imagePath in glob.glob("../discord/data/" + name + "/frames/*"):
        d,filename = os.path.split(imagePath)
        #filename = imagePath[imagePath.rfind("/") + 1:]
        image = cv2.imread(imagePath)
        #images[filename] = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        hist = cv2.calcHist([image], [0, 1, 2], None, [8, 8, 8],[0, 256, 0, 256, 0, 256])
        hist = cv2.normalize(hist, hist).flatten()
        #print("../discord/storage/his/frame/"+filename.split(".")[0]+".txt")
        np.savetxt("../discord/data/"+name+"/frames_txt/"+filename.split(".")[0]+".txt", hist)

    

    
    
    