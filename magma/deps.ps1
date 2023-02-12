# clone raylib: NOTE bleeding edge MASTER BRANCH
if (-not(Test-Path -Path "raylib")){
    git clone "https://github.com/raysan5/raylib" raylib
}

cd raylib
git checkout master
git fetch --prune
git merge
cd ..

#if (-not(Test-Path -Path "raygui")){
#    git clone "https://github.com/SasLuca/raygui.git" raygui
#}
if (-not(Test-Path -Path "box2d")){
    git clone --recursive "https://github.com/tobanteGaming/Box2D-cmake.git" box2d
}

cd box2d
git fetch --prune
git merge
cd ..
