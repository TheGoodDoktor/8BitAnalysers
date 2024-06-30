#pragma once

class FEmuBase;
struct FSkoolFileInfo;

/* 
   Import a skool file to use in the disassembly.
   - If a project is loaded the gamedata will be backed up.
   - A skoolinfo file will be saved, which saves info that cannot be stored in the code analysis data.
   - This skoolinfo file can be used when exporting a skool file to help replicate the original skool file.
   - If a project is loaded, the skoolinfo file will be written to the project root directory.
   - If no project is loaded it will be written to the workspace root.
   - By default the file will be called Out.skoolinfo. You can optionally override the name of the skoolinfo file
     with the pOutSkoolInfoName argument.
   - pSkoolInfo is optional. The skoolinfo data will be saved in pSkoolInfo if a pointer is passed in. 
*/
bool ImportSkoolFile(FEmuBase* pEmu, const char* pFilename, const char* pOutSkoolInfoName = nullptr, FSkoolFileInfo* pSkoolInfo = nullptr);

/* 
   Export a skool file to the game's workspace root directory. 
   - By default, the entire physical memory range will be output.
   - By default, the name of the skool file will be taken from the loaded project, or the filename can be passed 
     in optionally in pOutName.
   - If a skoolinfo file exists in the game's workspace directory it will be loaded. A skoolinfo file is optional.
     It's only useful when exporting a game previously imported from a skool file. See ImportSkoolFile()
*/
bool ExportSkoolFile(FEmuBase* pEmu, bool bHexadecimal, uint16_t startAddr, uint16_t endAddr, const char* pOutName = nullptr);