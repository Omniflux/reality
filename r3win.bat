@echo off

set cfg=Release
if "%1"=="debug" set cfg=Debug

if "%2"=="32" goto bitness32

echo Setting up the 64 bit version
copy /y src\Poser\addons\Reality\*.py "C:\Program Files\Smith Micro\Poser pro 2012\Runtime\Python\addons\Reality"
copy /y bin\w64\%cfg%\Reality.exe "C:\Program Files\Smith Micro\Poser pro 2012\Runtime\Python\addons\Reality"
echo Copying bin\w64\%cfg%\Reality_3.dll
copy /y bin\w64\%cfg%\Reality_3.dll "C:\Program Files\Smith Micro\Poser pro 2012\Runtime\Python\addons\Reality"
copy /y bin\w64\%cfg%\Reality.pyd "C:\Program Files\Smith Micro\Poser pro 2012\Runtime\Python\addons\Reality"

goto end

:bitness32
echo Setting up the 32 bit version
copy /y src\Poser\addons\Reality\*.py "C:\Program Files (x86)\Smith Micro\Poser pro 2012\Runtime\Python\addons\Reality"
copy /y bin\w32\%cfg%\Reality.exe "C:\Program Files (x86)\Smith Micro\Poser pro 2012\Runtime\Python\addons\Reality"
copy /y bin\w32\%cfg%\Reality_3.dll "C:\Program Files (x86)\Smith Micro\Poser pro 2012\Runtime\Python\addons\Reality"
copy /y bin\w32\%cfg%\Reality.pyd "C:\Program Files (x86)\Smith Micro\Poser pro 2012\Runtime\Python\addons\Reality"

:end