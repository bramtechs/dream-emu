#$EXE = ".\build\magma\tools\mgexplorer\Debug\mgexplorer.exe"
$EXE = ".\build\temple\Debug\temple.exe"
#$EXE = ".\build\dream\Debug\dream.exe"
if (-not(Test-Path -Path $EXE)){
    .\build.ps1
}
& $EXE
