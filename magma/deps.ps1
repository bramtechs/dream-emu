# clone raylib: NOTE bleeding edge MASTER BRANCH
if (-not(Test-Path -Path "raylib")){
    git clone "https://github.com/bramtechs/raylib" raylib
}
if (-not(Test-Path -Path "raygui")){
    git clone "https://github.com/bramtechs/raygui" raygui
}
