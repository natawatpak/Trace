# Trace
A simple discord reverse image lookup program for linux.

## Table of contents
* [License](#license)
* [Usage](#usage)
  * [Dependencies](#dependencies)
  * [Building](#building)
    * [Compile source files](#compile-source-files)
    * [Install Python module](#install-python-module)
    * [Launch Discord bot](#launch-discord-bot)
    * [Clean project](#clean-project)
* [Documentation](#documentation)
  * [Data structures](#data-structures)
  * [Functions](#functions)

## License
This program is under GPL-3.0 license.

## Usage
### Dependencies
* [python3](https://www.python.org/)
* [discord.py](https://github.com/Rapptz/discord.py)
* [cv2](https://pypi.org/project/opencv-python/)
* [numpy](https://numpy.org/)
* [gcc](https://gcc.gnu.org/)
* [makefile](https://www.gnu.org/software/make/)
### Building
#### Compile source files
To compile and run any examples in `examples/` or any file of choice.
```bash
make run DUMMY=path/to/file.c ARGS="arg1 arg2 ..."
```
#### Install Python module
To install the frame Python module provided by Trace
```bash
make module_install
```
#### Launch Discord bot
```bash
python3 bot.py
```
#### Clean project
Remove all objects and binary data.
```bash
make clean
```
## Documentation
### Data structures
### Functions

