void sendPlayerToWaitingList(Games *games, User user, int gameID, UserList *userList)
{
    cout << "wysylam gracza";
    GameDetails game = games->gamesList[gameID];

    cout << "Found game with matching ID. waiting list" << endl;

    // Find the game owner
    for (const User &gameUser : game.users)
    {
        cout << "Checking user in game with ID: " << gameUser.userID << endl;
        if (gameUser.userID == game.gameOwnerID && user.userID != game.gameOwnerID)
        {
            cout << "Found game owner with ID: " << game.gameOwnerID << endl;

            json jsonMessage;
            jsonMessage["type"] = "playerJoin";
            jsonMessage["player"] = user.nickname;

            sendComunicate(gameUser, jsonMessage, userList);

            cout << "Sending user ID " << user.userID << " to game owner with ID " << game.gameOwnerID << endl;
            // You can implement the actual sending logic here, for example, using sockets
            return;
        }
    }
    cout << "Game owner not found in users list for the game." << endl;

    cout << "Game not found for user with ID " << user.userID << endl;
}

void startGame(Games *games, json gameData, User user, UserList *userList)
{
    json jsonMessage;
    jsonMessage["type"] = "startGame";

    int gameID = gameData["gameID"].get<int>();

    jsonMessage["status"] = "started";

    games->gamesList[gameID].gameStatus = "started";

    sendComunicate(user, jsonMessage, userList); // send respond to owner
    sendQuestionsOrEndOfGame(games, gameID, false, userList);
}
