from count_func import call_count
import json
import discord
from discord.ext import commands
from last_week import recent
import Frame

client = discord.Client()

bot = commands.Bot(command_prefix='t!')

def read_token():
    with open("token.txt","r") as f:
        lines = f.readlines()
        return lines[0].strip()

token = read_token()

@bot.command()
async def source(ctx):
    await ctx.send('asdf')
    call_count('asdf')

@bot.command(name='top')
async def top(ctx):
    anime_list = json.load(open('frequency.json', 'r'))
    top = sorted(anime_list.items(), key=lambda x: x[1], reverse=True)
    for x in top:
        await ctx.send(f'{x}')


@bot.command(name='weekly')
async def weekly(ctx):
    for x in recent():
        await ctx.send(f'{x}')

@bot.command(name='add') 
async def add(ctx) :
    f = Frame.Frame() 
    client.

bot.run(token)
