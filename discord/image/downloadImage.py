import os
import random
import sys
import requests
import numpy as np

from image.Imagetohis import imagetohis 


localSaveRoot = '../discord/storage'

def downloadImage(url):
    print ("downloading: " + url)
    print (url)
    img_data = requests.get(url).content
    (d,name) = os.path.split(url)
    filePath = localSaveRoot +"/"+name
    with open(filePath, 'wb') as handler:
        handler.write(img_data)

    hist = imagetohis(url)
    np.savetxt("../discord/storage/his/"+name+".txt", hist)
    return 'success'

    
    #     return ret