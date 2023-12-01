@echo off
setlocal
pushd "%~dp0"

echo Deleting old package files...
if exist ..\Package rmdir /s /q ..\Package

mkdir ..\Package\Loose\Configs
mkdir ..\Package\Loose\GameData
mkdir ..\Package\Loose\Games
mkdir ..\Package\Loose\Docs
mkdir ..\Package\Loose\Fonts

echo Building CPCAnalyser.exe...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" 
if not exist build cmake -G Ninja -S ..\Source\CPC -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
echo copy exe...
if errorlevel 0 copy .\build\CPCAnalyser.exe ..\Package\Loose\

rem Add some files to describe what to put in the empty directories
@echo>"..\Package\Loose\Games\464 snapshots go here in sna format"
@echo>"..\Package\Loose\Fonts\fonts go here in ttf format"

echo copy imgui.ini...
copy ..\Data\CPCAnalyser\imgui.ini ..\Package\Loose\
echo copy logo...
copy ..\Data\CPCAnalyser\CPCALogo.png ..\Package\Loose\
echo copy docs...
copy ..\Docs\*.* ..\Package\Loose\Docs\

7z a ..\Package\CPCAnalyser.zip ..\Package\Loose\.

popd
endlocal
pause
