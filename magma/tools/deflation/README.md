# Deflation

Command-line tool to compress/"deflate" all your Raylib assets. Makes assets smaller and obfuscated to make ripping/data-mining _slightly_ harder.

Deflation excludes project files, like .blend, Aseprite and GIMP files.

// NOTE: check source to see what formats are excluded

Use the appropriate ``LoadCompressed*`` methods from ``magma/assets.h`` to import the generated files in-engine.

## Warning

Any subdirectories in your asset folder will be flattened for simplicity sake. Make sure no assets share the same name!

Instead of using sub-folders
```
sprites/player_left.png
sprites/player_right.png
music/spooky.mp3
```

you should name your files like this (GameMaker style)
```
spr_player_left.png
spr_player_right.png
mus_spooky.mp3
```
