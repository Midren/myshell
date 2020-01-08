# Shell 

## Table of contents
 - [Installing](#installing)
 - [Usage](#usage)
 
 
## Installing
Depends on the next C++ libraries:
- Curses

For Ubuntu:
```sh
sudo apt install libncurses-dev
```
Compile:
```
git clone https://github.com/RomanMilishchuk/myshell/
git submodule update --init --remote
mkdir build
cd build
cmake ..
make
```

## Usage
```
./myshell -- To run shell
./myshell **path** -- To run script
```
