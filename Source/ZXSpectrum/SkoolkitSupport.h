#pragma once

class FSpectrumEmu;
struct FSkoolFileInfo;

/* 
   Import a skool file to use in the disassembly.
   If a game is active the gamedata will be backed up.
   A skoolinfo file will be saved, which saves info that cannot be stored in the code analysis data.
   This skoolinfo file can be used when exporting to help replicate the original skool file.
   The name of the output skoolinfo file will be taken from the active game.
   If no game is active the filename of the output skoolinfo file must be passed in pOutSkoolInfoName.
   pSkoolInfo is optional. The skoolinfo data will be saved in pSkoolInfo if a pointer is passed in. 
*/
bool ImportSkoolFile(FSpectrumEmu* pEmu, const char* pFilename, const char* pOutSkoolInfoName = nullptr, FSkoolFileInfo* pSkoolInfo = nullptr);

/* 
   Export a skool file to the game's workspace root directory. The entire physical memory range will be output.
   By default the name of the skool file will be taken from the active game, or the filename can be passed 
   in optionally in pName.
   If a skoolinfo file exists in the game's workspace directory it will be loaded. A skoolinfo file is optional
   and only serves a purpose when dealing with a game that was previously imported from a skool file.
   A skoolinfo file will be written when importing a skool file, to help reproduce the format of the
   source skool file.
*/
bool ExportSkoolFile(FSpectrumEmu* pEmu, bool bHexadecimal, const char* pName = nullptr);

/* 
   Start a game, import a skool file and then export it, to test the SkoolKit importer and exporter are working 
   properly. You can optionally pass a game to start in pGameName. The output skool file will have the same name 
   as the game. If no game name is passed, then no game will be started and you must pass the name of the output 
   skool file in pOutSkoolName.
   The imported skool file will be loaded from the .\InputSkoolKit directory and written to the .\OutputSkoolKit 
   directory.
   This function can be used to import and export skool files for the spectrum rom. 
 */
 void DoSkoolKitTest(FSpectrumEmu* pEmu, const char* pGameName, const char* pInSkoolFileName, bool bHexadecimal, const char* pOutSkoolName = nullptr);