@echo off
setlocal
pushd "%~dp0"

echo Deleting old package files...
if exist ..\Package rmdir /s /q ..\Package

mkdir ..\Package\Loose\CPCGames
mkdir ..\Package\Loose\Docs
mkdir ..\Package\Loose\Fonts
mkdir ..\Package\Loose\Roms
mkdir ..\Package\Loose\Lua

echo copy exe...
if errorlevel 0 copy ..\Source\CPC\Build\Release\CPCAnalyser.exe ..\Package\Loose\

rem Add some files to describe what to put in the empty directories
@echo>"..\Package\Loose\CPCGames\464 or 6128 snapshots go here in sna format"
@echo>"..\Package\Loose\Fonts\fonts go here in ttf format"
rem @echo>"..\Package\Loose\Roms\external roms go here"

echo copy imgui.ini...
copy ..\Data\CPCAnalyser\imgui.ini ..\Package\Loose\
echo copy logo...
copy ..\Data\CPCAnalyser\CPCALogo.png ..\Package\Loose\
echo copy docs...
copy ..\Docs\*.* ..\Package\Loose\Docs\
echo copy Lua...
xcopy /e /v /y ..\Data\CPCAnalyser\Lua ..\Package\Loose\Lua\

"C:\Program Files\7-Zip\7z.exe" a ..\Package\CPCAnalyser.zip ..\Package\Loose\.

popd
endlocal
pause
