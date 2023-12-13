string nickNameStatus(Games *games, int gameID, string &gameNickname)
{
    // Check if the gameID is valid
    if (gameID < 0 || gameID >= static_cast<int>(games->gamesList.size()))
    {
        // Handle invalid gameID, e.g., throw an exception or return an error code
        return "error"; // error
    }

    // Iterate over the users in the specified game
    for (const User &user : games->gamesList[gameID].users)
    {
        if (user.nickname == gameNickname)
        {

            if (user.userID != -1)
                return "unavailable"; // cant rejoin user is connected to this nickname
            else
                return "rejoin"; // can rejoin user is not connectd to this nickname
        }
    }

    // The nickname is available
    return "available";
}
