@echo off

set PACKAGE_PATH=..\Package\CPC
set LOOSE_PATH=%PACKAGE_PATH%\Loose

echo Deleting old package files...
if exist %PACKAGE_PATH% rmdir /s /q %PACKAGE_PATH%

mkdir %LOOSE_PATH%\CPCGames
mkdir %LOOSE_PATH%\Docs
mkdir %LOOSE_PATH%\Fonts
rem mkdir %LOOSE_PATH%\Roms
mkdir %LOOSE_PATH%\Lua

echo copy exe...
if errorlevel 0 copy ..\Source\CPC\Build\Release\CPCAnalyser.exe %LOOSE_PATH%

rem Add some files to describe what to put in the empty directories
@echo>"%LOOSE_PATH%\CPCGames\464 or 6128 snapshots go here in sna format"
@echo>"%LOOSE_PATH%\Fonts\fonts go here in ttf format"
rem @echo>"%LOOSE_PATH%\Roms\external roms go here"

echo copy imgui.ini...
copy ..\Data\CPCAnalyser\imgui.ini %LOOSE_PATH%\
echo copy logo...
copy ..\Data\CPCAnalyser\CPCALogo.png %LOOSE_PATH%\
echo copy font...
copy ..\Data\CPCAnalyser\Fonts\Cousine-Regular.ttf %LOOSE_PATH%\Fonts\
echo copy docs...
copy ..\Docs\*.* %LOOSE_PATH%\Docs\
echo copy Lua...
xcopy /e /v /y ..\Data\CPCAnalyser\Lua %LOOSE_PATH%\Lua\

"C:\Program Files\7-Zip\7z.exe" a %PACKAGE_PATH%\CPCAnalyser.zip %LOOSE_PATH%\.

pause
