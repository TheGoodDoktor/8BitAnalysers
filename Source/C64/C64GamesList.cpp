#include "C64GamesList.h"

#include <Util/FileUtil.h>

bool FC64GamesList::EnumerateGames()
{
    FDirFileList listing;

    GamesList.clear();
    if (EnumerateDirectory("./Games", listing) == false)
        return false;

    for (const auto& file : listing)
    {
        const std::string& fn = file.FileName;
        if ((fn.substr(fn.find_last_of(".") + 1) == "prg") || (fn.substr(fn.find_last_of(".") + 1) == "PRG"))
        {
            FGameInfo game;
            game.Name = RemoveFileExtension(file.FileName.c_str());
            game.PRGFile = file.FileName;
            GamesList.push_back(game);
        }
    }
    return true;

}


int		FC64GamesList::DrawGameSelect()
{

    return -1;
}
