string handleAddingUserToGame(Games *games, json gameData, User user, UserList *userList)
{

    string status;

    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();
    string gameNickname = gameData["nickname"].get<string>();

    if (!games->gamesList.empty() && gameID >= 0 &&
        gameID < static_cast<int>(games->gamesList.size()) &&
        games->gamesList[gameID].gamePin == gamePin &&
        games->gamesList[gameID].gameStatus == "waitingForPlayers")

    {

        status = "success";
        user.nickname = gameNickname;
        games->gamesList[gameID].users.push_back(user);

        sendPlayerToWaitingList(games, user, gameID, userList);
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

string handleReJoin(Games *games, json gameData, User user)
{
    int gameID = gameData["gameID"].get<int>();
    string gameNickname = gameData["nickname"].get<string>();

    for (User &userFind : games->gamesList[gameID].users)
    {
        if (userFind.nickname == gameNickname)
        {
            userFind.userID = user.userID;
            break;
        }
    }

    return "reJoin";
}

void joinGame(Games *games, json gameData, User user, UserList *userList)
{
    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();
    string gameNickname = gameData["nickname"].get<string>();

    cout << gameID << endl;
    cout << gamePin << endl;

    json jsonMessage;
    jsonMessage["type"] = "joinGame";

    if (userNotInAnyGame(games, user))
    {

        string checkNickNameStatus = nickNameStatus(games, gameID, gameNickname);

        if (checkNickNameStatus == "unavailable")
        {
            jsonMessage["status"] = "nickNameNotAvailable";
        }
        else if (checkNickNameStatus == "rejoin")
        {
            jsonMessage["status"] = handleReJoin(games, gameData, user);
        }
        else if (checkNickNameStatus == "available")
        {
            // nick available
            jsonMessage["status"] = handleAddingUserToGame(games, gameData, user, userList);
        }
    }
    else
    {
        jsonMessage["status"] = "userAlreadyInGame";
    }

    sendComunicate(user, jsonMessage, userList);
}
