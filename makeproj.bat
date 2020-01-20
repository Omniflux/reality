@echo off

if "%1"=="" goto error

set compiler="Visual Studio 10"

if "%1"=="" (
  echo You must specify 32 or 64 for the type of build
  goto end
)

:: Make sure that we have the Qt base dir
if NOT exist C:\Qt md C:\Qt

if "%1"=="64" (
  setlocal
  call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /Release /x64 /win7
  set compiler="Visual Studio 10 Win64"
  set QT_SRC_DIR=qt-4.8.6-src-w64
) else (
  setlocal
  call  "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
  set QT_SRC_DIR=qt-4.8.6-bin-w32
)

:: Set the name for the destination directory where to copy Qt
set QT_DIR=C:\Qt\%QT_SRC_DIR%

if NOT exist %QT_DIR% (
  robocopy W:\devel\qt-4.8-windows\%QT_SRC_DIR% %QT_DIR% /mir
)

:: Set the configuration for Qt so that CMake's FindQt4 module works
:: We replace the \ with / as requested in the Qt configuration
echo [Paths] > %QT_DIR%\bin\qt.conf
echo Prefix = %QT_DIR:\=/% >> %QT_DIR%\bin\qt.conf

md build
md build\w%1
cd build\w%1

cmake -G %compiler% -DQT_QMAKE_EXECUTABLE="%QT_DIR%\bin\qmake.exe" -DBITNESS=%1 -DCMAKE_BUILD_TYPE=Release ..\..\src

cd ..\..

:end
endlocal
