import json
import datetime
import pytz

def call_count (name):
    return count(name)
    return day_count(name)


def count(name):
    try:
        anime_list = json.load(open('frequency.json', 'r'))
    except:
        anime_list = {}

    anime_list[name] = anime_list.get(name, 0) + 1

    result = json.dumps(anime_list, indent=1)
    with open("frequency.json", "w") as outfile:
        outfile.write(result)


def day_count(name):
    current_time = datetime.datetime.now(pytz.timezone('Asia/Bangkok')).strftime("%Y-%m-%d")

    try:
        weekly_anime_list = json.load(open('daily_frequency.json', 'r'))
    except:
        weekly_anime_list = {}
    weekly_anime_list[current_time] = weekly_anime_list.get(current_time, {})
    weekly_anime_list[current_time][name] = weekly_anime_list[current_time].get(name, 0) + 1

    weekly_result = json.dumps(weekly_anime_list, indent=1)
    with open("daily_frequency.json", "w") as outfile:
        outfile.write(weekly_result)
