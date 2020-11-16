import numpy as np
import argparse
import glob
import cv2
import os
import requests

from discord.image.Imagetohis import imagetohis 
index = {}
images = {}


def comparehis (url):
    
    query=imagetohis(url)

    min = 1000
    for story in glob.glob("../Trace/discord/data/*"):
        if not story.endswith(".json"):
            for his_Path in glob.glob(story+"/frames_txt/*"):
                    comhis = np.loadtxt(his_Path)
                    if chi2_distance(query,comhis)<min:
                        min = chi2_distance(query,comhis)
                        story_path = story
                        _,name = os.path.split(his_Path)
                #print(min)
    print(story_path)
    for img_Path in glob.glob(story_path+"/frames/"+name.split(".")[0] + ".*"):
        print(img_Path)
        return img_Path


def chi2_distance(histA, histB, eps = 1e-10):
    # compute the chi-squared distance
	d = 0.5 * np.sum([((a - b) ** 2) / (a + b + eps)
		for (a, b) in zip(histA, histB)])
	# return the chi-squared distance
	return d