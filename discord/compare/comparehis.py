#from scipy.spatial import distance as dist
#import matplotlib.pyplot as plt
import numpy as np
import argparse
import glob
import cv2
import os
import requests

from image.Imagetohis import imagetohis 
index = {}
images = {}


def comparehis (url):
    
    query=imagetohis(url)

    min = 1000
    for his_Path in glob.glob("../discord/storage" + "/his/*"):
        comhis = np.loadtxt(his_Path)
        if chi2_distance(query,comhis)<min:
            min = chi2_distance(query,comhis)
            d,path = os.path.split(his_Path)
            #print(min)
    return path


def chi2_distance(histA, histB, eps = 1e-10):
    # compute the chi-squared distance
	d = 0.5 * np.sum([((a - b) ** 2) / (a + b + eps)
		for (a, b) in zip(histA, histB)])
	# return the chi-squared distance
	return d