# forge
Easy to use cross-platform build system running on top of CMake, written in C++.

> Early in development UNTESTED ON LINUX
>
> Not related to [Minecraft Forge](https://files.minecraftforge.net/net/minecraftforge/forge/)!

## Dependencies
- git (command line)
- cmake
- cl or gcc supporting C++17

## Usage

- Add forge as a submodule to your existing CMake git project
```
git submodule add https://github.com/bramtechs/forge
```
- **IMPORTANT:** Edit project settings in ```forge.cpp```
- Compile with ```forge.bat``` or ```forge.sh```
- Run ```forge.exe [OPTION]``` from cmake root directory or forge subdirectory

### Example
- Look at the [forge-example](https://github.com/bramtechs/forge-example) repo!

- Using ```Native Tools Command Prompt For Visual Studio```:

```
> cd forge
> forge.bat
> forge.exe help

check      Check if required programs are installed
download   Clone required libraries from Github or merge new commits.
gen        Generate CMake project files
run        Run executable (debug)
runrel     Run executable (release)
generate   Generate CMake project files (alias)
package    Build and package optimized executable (NOT IMPLEMENTED)
build      Build project (for debugging)
release    Build optimized executable
clean      Remove build folder
wipe       clean + remove downloaded libraries
help       Show this screen
```

After running ```forge.bat``` once, you can run ```forge.exe``` or ```forge/forge.exe``` instead.

## TODO
- [ ] Automatically apply lib/build folders to .gitignore
- [ ] Add github workflow 
- [ ] Add release packaging support
