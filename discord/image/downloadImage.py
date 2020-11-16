import os
import random
import sys
import requests
import numpy as np

from discord.image.Imagetohis import imagetohis 


localSaveRoot = '../Trace/discord/data'

def downloadImage(ani_name,url):
    print ("downloading: " + url)
    print (url)
    img_data = requests.get(url).content
    (d,name) = os.path.split(url)

    if not os.path.exists("../Trace/discord/data/"+ ani_name):
        os.mkdir("../Trace/discord/data/"+ ani_name)
        os.mkdir("../Trace/discord/data/"+ ani_name+"/frames")
        os.mkdir("../Trace/discord/data/"+ ani_name+"/frames_txt")
    else:
        if not os.path.exists("../Trace/discord/data/"+ ani_name +"/frames"):
            os.mkdir("../Trace/discord/data/"+ ani_name+"/frames")
        if not os.path.exists("../Trace/discord/data/"+ ani_name +"/frames_txt"):
            os.mkdir("../Trace/discord/data/"+ ani_name+"/frames_txt")
    
    with open("../Trace/discord/data/"+ ani_name+"/frames/"+ name, 'wb') as handler:
        handler.write(img_data)
    
    hist = imagetohis(url)

    np.savetxt("../Trace/discord/data/"+ ani_name +"/frames_txt/" + name.split(".")[0]+".txt", hist)
    return 'success'

    
