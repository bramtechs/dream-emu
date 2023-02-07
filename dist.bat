WHERE cl
IF %ERRORLEVEL% NEQ 0 call shell.bat 

IF NOT EXIST dist mkdir dist 
pushd dist
cl /O2 /Ot ..\src\win32_magma.cpp user32.lib

IF NOT EXIST pack mkdir pack 
copy win32_magma.exe pack\win32_magma.exe
popd
