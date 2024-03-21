## Just enough Doom to run timedemo 3
![Doomtd3](readme_imgs/doomtd3.png?raw=true)

Does your computer have 320 kB free contiguous memory?
And is there a C compiler for it that supports 64-bit integers?

Then you can port Doom to it.
Well, enough parts of Doom to run timedemo 3.

32-bit CPU, 16-bit CPU.
Little-endian, big-endian.
It's all possible.

## How to add other platforms
Implement `i_system.h`. Look at `i_ibm.c` and `i_mac.c` for examples.

|Platform         |Platform specific code|Compiler                                               |Set environment variables|Compile code    |
|-----------------|----------------------|-------------------------------------------------------|-------------------------|----------------|
|IBM PC 16-bit    |`i_ibm.c`             |[gcc-ia16](https://github.com/tkchia/gcc-ia16)         |n/a                      |`compia16.sh`   |
|IBM PC 16-bit[^1]|`i_ibm.c`             |[Watcom](https://github.com/open-watcom/open-watcom-v2)|`setenvwc.bat`           |`compwc16.sh`   |
|IBM PC 32-bit    |`i_ibm.c`             |[DJGPP](https://github.com/andrewwutw/build-djgpp)     |`setenvdj.bat`           |`compdj.bat`    |
|Macintosh Plus   |`i_mac.c`             |[Retro68](https://github.com/autc04/Retro68)           |n/a                      |`CMakeLists.txt`|

[^1]: Two compilers can build the IBM PC 16-bit port. Gcc-ia16 produces faster code than Watcom. The static code analysers of both compilers detect different issues.
