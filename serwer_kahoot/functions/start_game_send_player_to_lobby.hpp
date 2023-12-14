void sendPlayerToWaitingList(Games *games, User user, int gameID, UserList *userList) // handle lobby 
{
    cout << "wysylam gracza";
    GameDetails game = games->gamesList[gameID];

    cout << "Found game with matching ID. waiting list" << endl;

    // Iterate through game's users
    for (const User &gameUser : game.users)
    {
        cout << "Checking user in game with ID: " << gameUser.userID << endl;
        if (gameUser.userID == game.gameOwnerID && user.userID != game.gameOwnerID) // Find owner
        {
            cout << "Found game owner with ID: " << game.gameOwnerID << endl;

            json jsonMessage;
            jsonMessage["type"] = "playerJoin";
            jsonMessage["player"] = user.nickname;

            sendComunicate(gameUser, jsonMessage, userList); // Send info about joining user to game owner

            cout << "Sending user ID " << user.userID << " to game owner with ID " << game.gameOwnerID << endl;

            return;
        }
    }
    cout << "Game owner not found in users list for the game." << endl;

    cout << "Game not found for user with ID " << user.userID << endl;
}

void startGame(Games *games, json gameData, User user, UserList *userList) // start game (stop waiting in lobby for players)
{
    json jsonMessage;
    jsonMessage["type"] = "startGame"; // Prepare json message 

    int gameID = gameData["gameID"].get<int>();

    jsonMessage["status"] = "started";

    games->gamesList[gameID].gameStatus = "started"; // Change game status

    sendComunicate(user, jsonMessage, userList); // Send respond to owner
    sendQuestionsOrEndOfGame(games, gameID, false, userList); // Send questions to players
}
