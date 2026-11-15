@echo off
rem Configure and build the Windows version with MSVC (Build Tools) + Ninja.
rem Usage: tools\build_windows.bat [Debug|Release]
setlocal
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

set VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat
rem (no parenthesised blocks here: the ")" in "(x86)" would close them)
if exist "%VCVARS%" goto have_vcvars
echo Visual Studio Build Tools 2026 not found: "%VCVARS%"
exit /b 1
:have_vcvars
call "%VCVARS%" >nul || exit /b 1

rem CMake and Ninja from the Android SDK, unless they are already on PATH.
where cmake >nul 2>nul || set "PATH=%LOCALAPPDATA%\Android\Sdk\cmake\3.30.4\bin;%PATH%"

cd /d "%~dp0.."
cmake -S . -B build\windows -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG% || exit /b 1
cmake --build build\windows || exit /b 1
echo Built build\windows\papalouie.exe
