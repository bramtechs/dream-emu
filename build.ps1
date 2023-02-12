.\deps.ps1

if (-not(Test-Path -Path "build")){
    .\gen.ps1
}


for ($i = 0; $i -le 10; $i++){ # add padding between builds
    Write-Host ""
}

if (Test-Path -Path C:\tools\censorship.exe){
    cmake --build build -j $Env:NUMBER_OF_PROCESSORS | C:\tools\censorship.exe
}
else{
    cmake --build build -j $Env:NUMBER_OF_PROCESSORS
}
