echo off

set PACKAGE_PATH=..\Package\ZX
set LOOSE_PATH=%PACKAGE_PATH%\Loose

echo Deleting old package files...
if exist %PACKAGE_PATH% rmdir /s /q %PACKAGE_PATH%

mkdir %LOOSE_PATH%\SpectrumGames
mkdir %LOOSE_PATH%\Pokes
mkdir %LOOSE_PATH%\RZX
mkdir %LOOSE_PATH%\Docs
mkdir %LOOSE_PATH%\Fonts
mkdir %LOOSE_PATH%\Lua

rem Add some files to describe what to put in the empty directories
@echo>"%LOOSE_PATH%\SpectrumGames\48k and 128k snapshots go here - z80 or sna"
@echo>"%LOOSE_PATH%\Pokes\pok files go here - filename matching game name"
@echo>"%LOOSE_PATH%\RZX\rzx files go here"
@echo>"%LOOSE_PATH%\Fonts\fonts go here in ttf format"

copy ..\Source\ZXSpectrum\build\bin\Release\SpectrumAnalyser.exe %LOOSE_PATH%\
copy ..\Data\SpectrumAnalyser\imgui.ini %LOOSE_PATH%\
copy ..\Data\SpectrumAnalyser\RomInfo.json %LOOSE_PATH%\
copy ..\Data\SpectrumAnalyser\SALogo.png %LOOSE_PATH%\
copy ..\Data\SpectrumAnalyser\Fonts\Cousine-Regular.ttf %LOOSE_PATH%\Fonts\
xcopy /e /v /y ..\Data\SpectrumAnalyser\Lua %LOOSE_PATH%\Lua\
copy ..\Docs\*.* %LOOSE_PATH%\Docs\

"c:\program files\7-Zip\7z.exe" a %PACKAGE_PATH%\SpectrumAnalyser.zip %LOOSE_PATH%\.

pause