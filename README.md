# Spectrum Analyser
![ci status](https://github.com/TheGoodDoktor/SpeccyExplorer/actions/workflows/ci.yml/badge.svg)
## Home Page
https://colourclash.co.uk/spectrum-analyser/
## Getting Started
### Clone project:
git clone --recursive https://github.com/TheGoodDoktor/8BitAnalysers
### Building
cd Source/ZXSpectrum\
mkdir build\
cd build\
cmake ..

Then for Linux/Mac:\
make

For windows a solution will be built in the build folder
## Running
Copy imgui.ini from the Data/SpectrumAnalyser folder into your working dir.
## Configuring
You will need to tell Spectrum Analyser where to find the files.\
When you first run the program a globalconfig.json will be created in the working dir. \
Edit the following fields in the globalconfig.json file:\
"WorkspaceRoot": "path/to/workspace"\
"SnapshotFolder": "path/to/snapshots"\
"PokesFolder": "path/to/pokes"

The workspace folder is where the analysis files will be stored.\
The snapshot folder is where you store your games (.z80, .sna)\
The pokes folder is where your .pok files are stored

# C64 Analyser
![ci status](https://github.com/TheGoodDoktor/SpeccyExplorer/actions/workflows/ci_c64.yml/badge.svg)\
This is very much work in progress and shouldn't be considered usable at this time....
