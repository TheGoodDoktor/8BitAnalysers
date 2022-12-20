

mkdir ..\Package\Loose\Configs
mkdir ..\Package\Loose\GameData
mkdir ..\Package\Loose\Games
mkdir ..\Package\Loose\Pokes
mkdir ..\Package\Loose\Docs

copy ..\VSProject\x64\Release\SpectrumAnalyser.exe ..\Package\Loose\
copy ..\Data\SpectrumAnalyser\imgui.ini ..\Package\Loose\

copy ..\Data\SpectrumAnalyser\Configs\Starquake.json ..\Package\Loose\Configs\
copy ..\Data\SpectrumAnalyser\GameData\RomInfo.bin ..\Package\Loose\GameData\
copy ..\Data\SpectrumAnalyser\GameData\Starquake.bin ..\Package\Loose\GameData\
copy ..\Data\SpectrumAnalyser\Pokes\Starquake.pok	..\Package\Loose\Pokes\
copy ..\Data\SpectrumAnalyser\Games\Starquake.z80	..\Package\Loose\Games\
copy ..\Docs\*.* ..\Package\Loose\Docs\

7z a ..\Package\SpectrumAnalyser.zip ..\Package\Loose\.
pause