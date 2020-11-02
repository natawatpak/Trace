import discord
from discord.ext import commands
import json
import datetime
import pytz

client = discord.Client()

bot = commands.Bot(command_prefix='t!')

def read_token():
    with open("token.txt","r") as f:
        lines = f.readlines()
        return lines[0].strip()

token = read_token()

@bot.command()
async def source(ctx):
    await ctx.send('fgh')
    call_count('fgh')


def call_count (name):
    count(name)
    weekly(name)


def count(name):
    try:
        anime_list = json.load(open('frequency.json', 'r'))
    except:
        anime_list = {}

    anime_list[name] = anime_list.get(name, 0) + 1

    result = json.dumps(anime_list, indent=1)
    with open("frequency.json", "w") as outfile:
        outfile.write(result)

'''t!source'''
'''t!count{asdasdasd}'''

def weekly(name):
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


@bot.command()
async def top5(ctx):
    anime_list = json.load(open('frequency.json', 'r'))
    top = sorted(anime_list.items(), key=lambda x: x[1], reverse=True)
    for x in top:
        await ctx.send(f'{x}')


@bot.command()
async def week_rank(ctx):
    weekly_anime_list = json.load(open('daily_frequency.json', 'r'))
    temp = {}
    for date, v in sorted(weekly_anime_list.items(), key=lambda x: x[0])[-7:]:
        for i, j in v.items():
            temp[i] = temp.get(i, 0) + j
    top = sorted(temp.items(), key=lambda x: x[1], reverse=True)
    for x in top:
        await ctx.send(f'{x}')

bot.run(token)

