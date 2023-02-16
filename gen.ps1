if (Test-Path -Path build){
    Remove-Item -Force -Recurse build
}
.\deps.ps1
 cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
