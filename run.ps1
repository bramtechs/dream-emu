$exe= ".\build\client\Debug\dreamemu_client.exe"

rm -Force $exe | Out-Null
cmake --build build -j $Env:NUMBER_OF_PROCESSORS
Write-Host $exe
.\build\client\Debug\dreamemu_client.exe
