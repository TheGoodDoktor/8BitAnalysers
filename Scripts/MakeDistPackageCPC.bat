echo on

echo Deleting old package files...
if exist ..\Package rmdir /s /q ..\Package

echo Building CPCAnalyser.exe...
if exist ..\Source\CPC\build\Release rmdir /s /q ..\Source\CPC\build\Release
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" ..\Source\CPC\build\CPCAnalyser.sln CPCAnalyser /Build Release

if not exist ..\Source\CPC\build\Release\CPCAnalyser.exe (
	echo Failed to build CPCAnalyser.exe. Aborting.
	exit /b
) 

mkdir ..\Package\Loose\Configs
mkdir ..\Package\Loose\GameData
mkdir ..\Package\Loose\Games
rem mkdir ..\Package\Loose\Games128
mkdir ..\Package\Loose\Docs
mkdir ..\Package\Loose\Fonts

rem Add some files to describe what to put in the empty directories
@echo>"..\Package\Loose\Games\464 snapshots go here in sna format"
@echo>"..\Package\Loose\Fonts\fonts go here in ttf format"

copy ..\Source\CPC\build\Release\CPCAnalyser.exe ..\Package\Loose\
copy ..\Data\CPCAnalyser\imgui.ini ..\Package\Loose\
copy ..\Data\CPCAnalyser\CPCALogo.png ..\Package\Loose\
copy ..\Docs\*.* ..\Package\Loose\Docs\

rem Make a batch file to run the 128k version
rem @echo CPCAnalyser.exe -128> "..\Package\Loose\CPCAnalyser128k.bat"

7z a ..\Package\CPCAnalyser.zip ..\Package\Loose\.

pause