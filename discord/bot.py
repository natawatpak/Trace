# bot.py
import os
import sys

import discord
from discord.ext import commands
from discord.ext.commands import Bot
from dotenv import load_dotenv
import asyncio
import json

from image.downloadImage import downloadImage
from compare.comparehis import comparehis

from count.count_func import call_count
from count.last_week import recent

load_dotenv()
TOKEN = os.getenv('DISCORD_TOKEN')

bot = Bot(command_prefix='!')
bot.remove_command('help')

@bot.command(name='server')
async def fetchServerInfo(context):
	guild = context.guild
	await context.send(f'Server Name: {guild.name}')
	await context.send(f'Server Size: {len(guild.members)}')
	await context.send(f'Server Name: {guild.owner.display_name}')

@bot.event
async def on_ready():
    print(f'{bot.user.name} has connected to Discord!')

@bot.event
async def on_member_join(member):
    await member.create_dm()
    await member.dm_channel.send(f'Hi {member.name}, welcome to my Discord server!')

@bot.command(name='help')
async def help(ctx):
    embed = discord.Embed(title="Trace's available commands",
                          description='',
                          colour=discord.Color.blue()
                          )

    #embed.set_author(name=client.user.name, icon_url='https://discordpy.readthedocs.io/en/latest/_images/snake.png')
    embed.set_footer(text='Hope this helps!', icon_url='https://pbs.twimg.com/media/DcEu95UWAAIvoWP.png')
    #embed.set_image(url='https://pbs.twimg.com/media/DcEu9_wW0AAE2VK.png')
    embed.set_thumbnail(url='https://pbs.twimg.com/media/DcEu9_bXcAAQ-Cv.png')

    embed.add_field(name="Find source  :face_with_raised_eyebrow: ", value="t!source <picture url>")
    embed.add_field(name="Top 5 source found!  :pencil: ", value="t!top", inline=False)
    embed.add_field(name="Add source! ", value="t!add <picture/video url>", inline=False)

    await ctx.send(embed=embed)

@bot.command(name='annouce')
async def announce(ctx, channel: discord.TextChannel, *, msg):
    await ctx.send('Success!')
    await channel.send(f'{msg}')

pic_ext = ['.jpg','.png','.jpeg']

@bot.command(name='add')
async def add(ctx, url):
    run = False
    for ext in pic_ext:
        if url.endswith(ext):
            await ctx.send('picture')
            await ctx.send(str(ctx.author.mention)+" \n"+ downloadImage(url))
            run = True
            break
    
    if not run :
        await ctx.send('command error')

@bot.command(name='source')
async def source(ctx, url):
    run = False
    for ext in pic_ext:
        if url.endswith(ext):
            await ctx.send('picture')
            source_name = comparehis(url)
            await ctx.send(str(ctx.author.mention)+" \n"+ source_name)
            call_count(source_name)
            run = True
            break

    if not run :
        await ctx.send('command error')

@bot.command(name='top')
async def top(ctx):
    anime_list = json.load(open('storage/frequency.json', 'r'))
    top = sorted(anime_list.items(), key=lambda x: x[1], reverse=True)
    for x in top:
        await ctx.send(f'{x}')

@bot.command(name='weekly')
async def weekly(ctx):
    for x in recent():
        await ctx.send(f'{x}')

bot.run(TOKEN)