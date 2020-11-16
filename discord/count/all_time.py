from discord.count.count_func import call_count
import json

def forever():
    anime_list = json.load(open('../Trace/discord/data/frequency.json', 'r'))
    popular = sorted(anime_list.items(), key=lambda x: x[1], reverse=True)
    return popular