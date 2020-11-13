from count.count_func import call_count
import json

def recent():
    weekly_anime_list = json.load(open('storage/daily_frequency.json', 'r'))
    week = {}
    for date, v in sorted(weekly_anime_list.items(), key=lambda x: x[0])[-7:]:
        for i, j in v.items():
            week[i] = week.get(i, 0) + j
    top_week = sorted(week.items(), key=lambda x: x[1], reverse=True)[:5]
    return top_week