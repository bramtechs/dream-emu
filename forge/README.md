# forge
Easy to use cross-platform build system on top of CMake, written in C++.

> Not related to [Minecraft Forge](https://files.minecraftforge.net/net/minecraftforge/forge/)!

## Dependencies
- git (command line)
- cmake
- cl or gcc supporting C++17

## Usage

- Add forge as a submodule in your existing CMake git project
- Edit project settings in ```forge.cpp```
- Compile with ```forge.bat``` or ```forge.sh```
- Run ```forge.exe [OPTION]``` from cmake root directory or forge subdirectory

### Example
Using ```Native Tools Command Prompt For Visual Studio```:

```
> cd forge
> forge.bat
> forge.exe help

check      Check if required programs are installed (TODO)
download   Clone required libraries from Github or merge new commits.
gen        Generate CMake project files
generate   Generate CMake project files (alias)
build      Build project (for debugging)
release    Build optimized executable
package    Build and package optimized executable (TODO)
run        Run executable
wipe       Remove all cloned libraries (use if things broke)
clean      Remove build folder
help       Show this screen
```

## TODO
- [ ] Add packaging support
- [ ] Add tool checking
- [ ] Add generator settings
