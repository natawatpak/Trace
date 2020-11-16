import os
import random
import sys
import requests
import numpy as np

from discord.image.Imagetohis import imagetohis 


localSaveRoot = '../discord/data'

def downloadImage(ani_name,url):
    print ("downloading: " + url)
    print (url)
    img_data = requests.get(url).content
    (d,name) = os.path.split(url)

    if not os.path.exists("../discord/data/"+ ani_name):
        os.mkdir("../discord/data/"+ ani_name) 
        os.mkdir("../discord/data/"+ ani_name+"/frames") 
        os.mkdir("../discord/data/"+ ani_name+"/frames_txt") 
    else:
        if not os.path.exists("../discord/data/"+ ani_name +"/frames"):
            os.mkdir("../discord/data/"+ ani_name+"/frames") 
        if not os.path.exists("../discord/data/"+ ani_name +"/frames_txt"):
            os.mkdir("../discord/data/"+ ani_name+"/frames_txt") 
    
    with open("../discord/data/"+ ani_name+"/frames/"+ name, 'wb') as handler:
        handler.write(img_data)
    
    hist = imagetohis(url)

    np.savetxt("../discord/data/"+ ani_name +"/frames_txt/" + name.split(".")[0]+".txt", hist)
    return 'success'

    
