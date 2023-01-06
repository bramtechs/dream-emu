# clone engine: NOTE bleeding edge
if (-not(Test-Path -Path "magma")){
    git clone "https://github.com/bramtechs/RaylibMagmaEngine" magma
}
cd magma; ./deps.ps1; cd ..
