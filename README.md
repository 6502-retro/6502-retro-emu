<!-- vim: set tw=80 sw=4 ts=4 et cc=80: -->
# 6502 Retro! Emulator

## About This Emulator

This is a functional emulator.  It's about the simplest type possible.  In
otherwords, this is not a cycle accurate emulator.  It's just enough emulation
to test the filesystem and console (UART) features of the 6502-Retro!.

And by UART, I really just mean the basic input and output.  No flow control or
any other hardware specific features.  To be fair, I never got flow control
working anyway.

## Usage

```text
emu [<flags>] [command] [args]:
  -h             this help
  -d             enter debugger on startup
  -t             enable instruction tracing on startup
If command is specified, a Unix file of that name will be loaded and
injected directly into memory (it's not loaded through the CCP).
The first two arguments are mapped to the standard FCBs.

CTRL+C will usually issue a CTRL+C to the program.
CTRL+X will exit to the debugger.
```

There is a `make_sdcard.sh` script in the
[6502-retro-os](https://github.com/6502-retro/6502-retro-os.git) project that
will create an sdcard image with the default apps on drive A:

## Development

Because this emulator emulates the BIOS only, there will have to be some kind of
OS on the sdcard image to work with.  So there are two main choices:

1. put the code you're developping into sectors 1 to 16 (inclusive) of the
   sdcard image.  This is where the OS is copied to by the `make_sdcard.sh` script
   above. This is what you should do if you're writing a new OS.  The emulator
   automatically jumps to the address at FFFC.
   1. This is not actually true right now.  It should, but the 6502-Retro-Os has
      sdcard initialization which hangs in the emulator.  A magic number is used
      as the start address for now.  I am thinking of approaches to override
      this address.  Perhaps with a commandline option.
2. use the `py_sfs_v2/cli.py` script to copy your compiled `.com` files into one
   of the 8 drives on the sdcard image.  Then boot the emulator.

_note: there is some work to be done on the `py_sfs_v2/cli.py` script to support
overwriting files copied to the sdcard image.  I think the intention was that it
should be doing this, but in latest testing, it does not.  So I think that's a
bug.  For now, just `era your.com` file inside the OS within the emulator._

## Running

Simply run the emulator and path to this image to get started.

```text
./emu ../6502-retro-os/py_sfs_v2/6502-retro-sdcard.img
```

## Debugging

Press `CTRL+X` to enter the debugger.

```text
A>PC=020C SP=01FC A=80 X=03 Y=01 P=01 -------C
020c : 4c 34 ee : jmp EE34
debug>?
Sleazy debugger
  r               show registers
  r <reg> <value> set register
  b               show breakpoints
  b <addr>        set breakpoint
  db <addr>       delete breakpoint
  w <addr>        set watchpoint
  dw <addr>       delete watchpoint
  m <addr> <len>  show memory
  u <addr> <len>  unassemble memory
  s               single step
  g               continue
  q               quit
  trace 0|1       enable tracing
```

If your code has a `brk` statement in it, then the debugger will automatically
enter debug mode at that address.

## Build

```text
make
```

## LICENSE

This work is dirrived from work by David Given here:
[https://github.com/davidgiven/cpm65.git](https://github.com/davidgiven/cpm65.git)

Specifically the `cpmemu` code which includes the modifications to the LIB6502
and reference architecture for this emulator.

As such, I have taken care to use the same license for this project.

### LICENSE TEXT

BSD 2-Clause License

Copyright (c) 2025, David Latham
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## LIB6502 LICENSE

`lib6502` contains a hacked copy of the lib6502 library, which is ©2005 Ian
Plumarta and is available under the terms of the MIT license. See
`COPYING.lib6502` for the full text.
