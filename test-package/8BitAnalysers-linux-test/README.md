# 8-Bit Analysers
A set of tools that assist the analysis & annotation of 8-bit games running on a selection of 8-bit systems.

## Download Latest Build
[![Weekly Builds](https://github.com/TheGoodDoktor/8BitAnalysers/actions/workflows/weekly-builds.yml/badge.svg)](https://github.com/TheGoodDoktor/8BitAnalysers/actions/workflows/weekly-builds.yml)
[![Latest Release](https://img.shields.io/github/v/release/TheGoodDoktor/8BitAnalysers?include_prereleases&label=latest%20build)](https://github.com/TheGoodDoktor/8BitAnalysers/releases/latest)

**[⬇️ Download Latest Windows Build](https://github.com/TheGoodDoktor/8BitAnalysers/releases/latest/download/8BitAnalysers-windows.zip)**

**[⬇️ Download Latest Linux Build](https://github.com/TheGoodDoktor/8BitAnalysers/releases/latest/download/8BitAnalysers-linux.tar.gz)**

**[⬇️ Download Latest macOS Build](https://github.com/TheGoodDoktor/8BitAnalysers/releases/latest/download/8BitAnalysers-macos.tar.gz)**

Builds are automatically generated every week and include pre-compiled binaries for all supported analysers.

## Wiki
https://github.com/TheGoodDoktor/8BitAnalysers/wiki
## Getting Started
### Clone project:
git clone --recursive https://github.com/TheGoodDoktor/8BitAnalysers
# Spectrum Analyser
![ci status](https://github.com/TheGoodDoktor/SpeccyExplorer/actions/workflows/ci.yml/badge.svg)
## Home Page
https://colourclash.co.uk/spectrum-analyser
## Building
cd Source/ZXSpectrum\
mkdir build\
cd build\
cmake ..

Then for Linux/Mac:\
make\
To build an XCode project for Mac:\
cmake -G Xcode ..

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
This project is usable but behind Spectrum Analyser.\
At the moment only PRG files are supported with TAP support on the way.
## Building, Running & Configuring
This is similar to Spectrum Analyser but with the C64 directory.\
For example to build:\
cd Source/C64\
mkdir build\
cd build\
cmake ..

# CPC Analyser
![ci status](https://github.com/TheGoodDoktor/SpeccyExplorer/actions/workflows/ci_cpc.yml/badge.svg)\
This is usable but may contain missing or incomplete features compared to Spectrum Analyser.
### Home Page
https://colourclash.co.uk/cpc-analyser/
