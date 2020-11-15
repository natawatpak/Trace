# bot.py
import os
import sys

import discord
from discord.ext import commands
from discord.ext.commands import Bot
from dotenv import load_dotenv
import asyncio
import json

from discord.image.downloadImage import downloadImage
from discord.compare.comparehis import comparehis


from discord.count.count_func import call_count
from discord.count.last_week import recent

load_dotenv()
TOKEN = os.getenv('DISCORD_TOKEN')

bot = Bot(command_prefix='t!')
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
async def add(ctx, name, url):
    run = False

    for ext in pic_ext:
        if url.endswith(ext):
            await ctx.send('picture')
            await ctx.send(str(ctx.author.mention)+" \n"+ downloadImage(name,url))
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
            source_path = comparehis(url)
            print(source_path)
            d, _ = os.path.split(source_path)
            d, _= os.path.split(d)
            _,name= os.path.split(d)
            call_count(name)
            await ctx.send(str(ctx.author.mention)+" \n"+ name)
            info = json.load(open(d+'/meta.json', 'r'))
            file = discord.File(source_path, filename= "image"+"."+source_path.split(".")[-1])
            print(source_path.split(".")[-1])
            embed = discord.Embed(
                title=info["QUERY_FLAG_ROMANJI_NAME"]+"\n"+info["QUERY_FLAG_ENGLISH_NAME"],
                description=info["QUERY_FLAG_YEAR"],
                colour=discord.Color.blue()
            )
            embed.add_field(name="Episode", value=info["QUERY_FLAG_EPISODES"], inline=False)
            embed.add_field(name="Ratings", value=info["QUERY_FLAG_RATINGS"], inline=False)
            embed.add_field(name="Tags", value=info["QUERY_FLAG_TAG_NAME_LIST"].replace(",",", "),inline=False)
            embed.add_field(name="\a", value= '\a')
            embed.add_field(name="Best matched", value= '\a', inline=False)
            embed.set_image(url="attachment://"+"image"+"."+source_path.split(".")[-1])
            await ctx.send(file=file,embed=embed)
            run = True
            break

    if not run :
        await ctx.send('command error')

@bot.command(name='top')
async def top(ctx):
    anime_list = json.load(open('data/frequency.json', 'r'))
    top = sorted(anime_list.items(), key=lambda x: x[1], reverse=True)
    for x in top:
        await ctx.send(f'{x}')

@bot.command(name='weekly')
async def weekly(ctx):
    for x in recent():
        await ctx.send(f'{x}')

bot.run(TOKEN)