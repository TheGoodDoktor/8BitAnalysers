#include "C64GamesList.h"

#include <Util/FileUtil.h>
#include <imgui.h>

bool FC64GamesList::EnumerateGames(const char *pDirectory)
{
    FDirFileList listing;
    
    SelectedGame = -1;
    GamesList.clear();
    if (EnumerateDirectory(pDirectory, listing) == false)
        return false;

    for (const auto& file : listing)
    {
        const std::string& fn = file.FileName;
        if ((fn.substr(fn.find_last_of(".") + 1) == "prg") || (fn.substr(fn.find_last_of(".") + 1) == "PRG"))
        {
            FGameInfo game;
            game.Name = RemoveFileExtension(file.FileName.c_str());
            game.PRGFile = std::string(pDirectory) + file.FileName;
            GamesList.push_back(game);
        }
    }
    return true;

}

const FGameInfo* FC64GamesList::GetGameInfo(const char* pName)
{
    for (int gameNo = 0; gameNo < (int)GamesList.size(); gameNo++)
    {
        if(GamesList[gameNo].Name == pName)
            return &GamesList[gameNo];
    }

    return nullptr;
}


int		FC64GamesList::DrawGameSelect()
{
    for (int gameNo = 0; gameNo < (int)GamesList.size(); gameNo++)
    {
        if (ImGui::Selectable(GamesList[gameNo].Name.c_str(),SelectedGame == gameNo))
        {
            SelectedGame = gameNo;
            return SelectedGame;
        }
    }
    return -1;
}
