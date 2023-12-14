void startWatingForPlayer(Games *games, json gameData, User user, UserList *userList) // Start lobby
{
    if (userNotInAnyGame(games, user)) // Check if user how want to start lobby is not in any other game 
    {
        int gameID = gameData["gameID"].get<int>();

        if (games->gamesList[gameID].gameOwnerID != -1) // Check if game hadn't had any previous owner
        {
            handleStartWithOutRejoin(games, gameData, user, userList); // Handle starting lobby without rejoining
            cout << " wlasciciela" << games->gamesList[gameID].gameOwnerID << endl;
        }
        else // Game had previous owner
            handleStartWithRejoin(games, gameData, user, userList); // Handle starting lobby with rejoining
    }
    else // User is in other game send communicate
    {
        json jsonMessage;
        jsonMessage["type"] = "startWatingForPlayer";
        jsonMessage["status"] = "userAlreadyInGame";

        sendComunicate(user, jsonMessage, userList);
    }
}