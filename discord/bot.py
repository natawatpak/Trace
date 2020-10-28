# bot.py
import os
import random
import sys

import discord
from discord.ext.commands import Bot
from dotenv import load_dotenv
import asyncio

from image.downloadImage import downloadImage

load_dotenv()
TOKEN = os.getenv('DISCORD_TOKEN')

# bot = Bot(command_prefix='!')
client = discord.Client()

# @bot.command(name='server')
# async def fetchServerInfo(context):
# 	guild = context.guild
# 	await context.send(f'Server Name: {guild.name}')
# 	await context.send(f'Server Size: {len(guild.members)}')
# 	await context.send(f'Server Name: {guild.owner.display_name}')


@client.event
async def on_ready():
    print(f'{client.user.name} has connected to Discord!')

@client.event
async def on_member_join(member):
    await member.create_dm()
    await member.dm_channel.send(f'Hi {member.name}, welcome to my Discord server!')

pic_ext = ['.jpg','.png','.jpeg']

@client.event
async def on_message(message):
    if message.author == client.user:
        return

    brooklyn_99_quotes = [
        'I\'m the human form of the 💯 emoji.',
        'Bingpot!',
        (
            'Cool. Cool cool cool cool cool cool cool, '
            'no doubt no doubt no doubt no doubt.'
        ),
    ]

    #print(message.attachments[0])
    # print(message.attachments[0].url)
    
    if message.content == '99!':
        response = random.choice(brooklyn_99_quotes)
        await message.channel.send(response)
    
    if len(message.attachments) != 0 :
        url = message.attachments[0].url
        for ext in pic_ext:
            if url.endswith(ext):
                await message.channel.send('picture')
                await message.channel.send(str(message.author.name)+" \n"+ downloadImage(url))

client.run(TOKEN)