echo on

echo Deleting old package files...
if exist ..\Package rmdir /s /q ..\Package

mkdir ..\Package\Loose\Prg
mkdir ..\Package\Loose\Docs
mkdir ..\Package\Loose\Fonts

rem Add some files to describe what to put in the empty directories
@echo>"..\Package\Loose\Prg\C64 prg files go here"
@echo>"..\Package\Loose\Fonts\fonts go here in ttf format"

copy ..\Source\C64\build\bin\Release\C64Analyser.exe ..\Package\Loose\
copy ..\Data\C64Analyser\imgui.ini ..\Package\Loose\
copy ..\Data\C64Analyser\Fonts\Cousine-Regular.ttf ..\Package\Loose\Fonts\
rem copy ..\Data\C64Analyser\SALogo.png ..\Package\Loose\
copy ..\Docs\*.* ..\Package\Loose\Docs\

7z a ..\Package\C64Analyser.zip ..\Package\Loose\.

pause