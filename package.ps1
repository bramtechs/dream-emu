$BUILD_DIR= ".\build\dream\Release"
$ErrorActionPreference = "Stop"

# remove old build
if (Test-Path -Path "build"){
    rm .\build -r -Force
}

# make ZIP release
$logName = Get-Date -UFormat ".\releases\DREAM_EMU-%Y-%m-%d-%H-%M_log.txt"
$fileName = Get-Date -UFormat ".\releases\DREAM_EMU-%Y-%m-%d-%H-%M.ZIP"
mkdir -Force releases
.\dist.ps1 | Out-File $logName
Write-Host "Built done!"
Compress-Archive -Force -Path $BUILD_DIR\* -DestinationPath $fileName
