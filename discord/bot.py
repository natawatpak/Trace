# bot.py
import os
import sys

import discord
from discord.ext import commands
from discord.ext.commands import Bot
from dotenv import load_dotenv
import asyncio

from image.downloadImage import downloadImage
from compare.comparehis import comparehis

load_dotenv()
TOKEN = os.getenv('DISCORD_TOKEN')

bot = Bot(command_prefix='!')

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

pic_ext = ['.jpg','.png','.jpeg']

@bot.command(name='add', help = 'create picture data')
async def source(ctx, url):
    run = False
    for ext in pic_ext:
        if url.endswith(ext):
            await ctx.send('picture')
            await ctx.send(str(ctx.author.mention)+" \n"+ downloadImage(url))
            run = True
            break
    
    if not run :
        await ctx.send('command error')

@bot.command(name='source', help = 'find picture source')
async def source(ctx, url):
    run = False
    for ext in pic_ext:
        if url.endswith(ext):
            await ctx.send('picture')
            await ctx.send(str(ctx.author.mention)+" \n"+ comparehis(url))
            run = True
            break

    if not run :
        await ctx.send('command error')

bot.run(TOKEN)