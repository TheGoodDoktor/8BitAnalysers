echo off

set PACKAGE_PATH=..\Package\C64
set LOOSE_PATH=%PACKAGE_PATH%\Loose

echo Deleting old package files...
if exist %PACKAGE_PATH% rmdir /s /q %PACKAGE_PATH%

mkdir %LOOSE_PATH%\Prg
mkdir %LOOSE_PATH%\Docs
mkdir %LOOSE_PATH%\Fonts

rem Add some files to describe what to put in the empty directories
@echo>"%LOOSE_PATH%\Prg\C64 prg files go here"
@echo>"%LOOSE_PATH%\Fonts\fonts go here in ttf format"

copy ..\Source\C64\build\bin\Release\C64Analyser.exe %LOOSE_PATH%\
copy ..\Data\C64Analyser\imgui.ini %LOOSE_PATH%\
copy ..\Data\C64Analyser\Fonts\Cousine-Regular.ttf %LOOSE_PATH%\Fonts\
rem copy ..\Data\C64Analyser\SALogo.png %LOOSE_PATH%\
copy ..\Docs\*.* %LOOSE_PATH%\Docs\

7z a %PACKAGE_PATH%\C64Analyser.zip %LOOSE_PATH%\.

pause