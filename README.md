# Gameboy Emulator
This is a small project to understand the architecture of a gameboy and try to replicate it using C.

## Dependencies
The `libsdl2`, `cmake` and `check` dependencies are required to develop and build this project.

## Build and Run
To build, create a directory called `/build` in the root of the project. From inside the directory, run `cmake ..` in order create the build files.

Running `make` from within the `/build` directory can then be run to generate and link the executable. The executable can then be found at `build/gbemu/gbemu`.

You may use the supplied ROMs in order to execute and test the emulator.
These can be found at `/roms`.