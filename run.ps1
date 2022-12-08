$exe= ".\build-win\client\Debug\dreamemu_client.exe"

rm -Force $exe | Out-Null
cmake --build build-win -j $Env:NUMBER_OF_PROCESSORS
Write-Host $exe
& { Start-Process -FilePath $exe }
