string handleAddingUserToGame(Games *games, json gameData, User user, UserList *userList)
{

    string status;

    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();
    string gameNickname = gameData["nickname"].get<string>();

    if (!games->gamesList.empty() && gameID >= 0 &&
        gameID < static_cast<int>(games->gamesList.size()) &&
        games->gamesList[gameID].gamePin == gamePin &&
        games->gamesList[gameID].gameStatus == "waitingForPlayers") // Check if game is available 

    {

        status = "success";
        user.nickname = gameNickname;
        games->gamesList[gameID].users.push_back(user); // Add users to game 

        sendPlayerToWaitingList(games, user, gameID, userList); // Send user to lobby (owner see who joined the game)
    }
    else if (!games->gamesList.empty() && games->gamesList[gameID].gameStatus == "started")
    {
        status = "started";
    }
    else
    {
        status = "fail";
    }

    return status;
}

string handleReJoin(Games *games, json gameData, User user) // handle rejoin to game (only players)
{
    int gameID = gameData["gameID"].get<int>();
    string gameNickname = gameData["nickname"].get<string>();

    for (User &userFind : games->gamesList[gameID].users) // Iterate through games 
    {
        if (userFind.nickname == gameNickname) // Find available nickname to rejoin 
        {
            userFind.userID = user.userID;
            break;
        }
    }

    return "reJoin";
}

void joinGame(Games *games, json gameData, User user, UserList *userList) // Join game (only players)
{
    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();
    string gameNickname = gameData["nickname"].get<string>();

    cout << gameID << endl;
    cout << gamePin << endl;

    json jsonMessage;
    jsonMessage["type"] = "joinGame";

    if (userNotInAnyGame(games, user)) // Check if user is not in any other game 
    {

        string checkNickNameStatus = nickNameStatus(games, gameID, gameNickname); // Check nickname status

        if (checkNickNameStatus == "unavailable")
        {
            //Nickname not available
            jsonMessage["status"] = "nickNameNotAvailable";
        }
        else if (checkNickNameStatus == "rejoin")
        {
            // Can rejoin to this nickname
            jsonMessage["status"] = handleReJoin(games, gameData, user);
        }
        else if (checkNickNameStatus == "available")
        {
            // Nickname available
            jsonMessage["status"] = handleAddingUserToGame(games, gameData, user, userList);
        }
    }
    else
    {
        jsonMessage["status"] = "userAlreadyInGame";
    }

    sendComunicate(user, jsonMessage, userList); // Send message to user
}
