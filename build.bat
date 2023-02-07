@echo off
rem launch this from msvc-enabled console

WHERE cl
IF %ERRORLEVEL% NEQ 0 call shell.bat 

pushd magma
call build.bat
popd

pushd temple
call build.bat
popd
