@ECHO OFF
REM Set the target platform for the compiler (Ex: x86 or x64)
set TARGET_PLATFORM=x86

WHERE cl >nul 2>nul
IF %ERRORLEVEL% == 0 goto ARGS
REM Activate the msvc build environment if cl isn't available yet
IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
) ELSE IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  set VC_INIT="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat"
) ELSE (
  echo "Couldn't find vcvarsall.bat or vcbuildtools.bat, please set it manually."
  exit /B
)
goto ARGS

:HELP
echo Usage: build.bat [-hcd]
echo  -h  Show this information
echo  -d  Debug build
exit /B

:ARGS
setlocal EnableDelayedExpansion
setlocal EnableExtensions

set ARG=%1
if "!ARG!" == "" ( goto BUILD )
IF NOT "x!ARG!" == "x!ARG:h=!" (
  goto HELP
)
IF NOT "x!ARG!" == "x!ARG:d=!" (
  set BUILD_DEBUG=1
)
IF NOT "x!ARG!" == "x!ARG:u=!" (
  set UPX_IT=1
)
IF NOT "x!ARG!" == "x!ARG:r=!" (
  set RUN_AFTER_BUILD=1
)

:BUILD
call %VC_INIT% %TARGET_PLATFORM% > NUL 2>&1

IF DEFINED BUILD_DEBUG (
    set COMP_FLAGS=/std:c++20 /O2 /GL /EHsc /DEBUG
) ELSE (
    set COMP_FLAGS=/std:c++20 /O2 /GL /EHsc
)

cl.exe palgen.cpp %COMP_FLAGS%
