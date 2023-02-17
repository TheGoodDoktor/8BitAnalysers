

mkdir ..\Package\Loose\Configs
mkdir ..\Package\Loose\GameData
mkdir ..\Package\Loose\Games
mkdir ..\Package\Loose\Pokes
mkdir ..\Package\Loose\Docs

copy ..\Source\ZXSpectrum\build\Release\SpectrumAnalyser.exe ..\Package\Loose\
copy ..\Data\SpectrumAnalyser\imgui.ini ..\Package\Loose\
copy ..\Data\SpectrumAnalyser\RomInfo.json ..\Package\Loose\
copy ..\Data\SpectrumAnalyser\SALogo.png ..\Package\Loose\
copy ..\Docs\*.* ..\Package\Loose\Docs\

7z a ..\Package\SpectrumAnalyser.zip ..\Package\Loose\.
pause