$EXE = ".\build\dream\Debug\dream.exe"
if (-not(Test-Path -Path $EXE)){
    .\build.ps1
}
& $EXE
