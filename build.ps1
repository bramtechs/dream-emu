.\deps.ps1

if (-not(Test-Path -Path "build")){
    .\gen.ps1
}


#if (Test-Path -Path C:\tools\censorship.exe){
#    cmake --build build -j $Env:NUMBER_OF_PROCESSORS | C:\tools\censorship.exe
#}
#else{
    cmake --build build -j $Env:NUMBER_OF_PROCESSORS
#}
