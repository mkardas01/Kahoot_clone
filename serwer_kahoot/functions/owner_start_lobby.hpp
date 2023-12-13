void startWatingForPlayer(Games *games, json gameData, User user, UserList *userList)
{
    if (userNotInAnyGame(games, user))
    {
        int gameID = gameData["gameID"].get<int>();

        if (games->gamesList[gameID].gameOwnerID != -1)
        {
            handleStartWithOutRejoin(games, gameData, user, userList);
            cout << " wlasciciela" << games->gamesList[gameID].gameOwnerID << endl;
        }
        else
            handleStartWithRejoin(games, gameData, user, userList);
    }
    else
    {
        json jsonMessage;
        jsonMessage["type"] = "startWatingForPlayer";
        jsonMessage["status"] = "userAlreadyInGame";

        sendComunicate(user, jsonMessage, userList);
    }
}
