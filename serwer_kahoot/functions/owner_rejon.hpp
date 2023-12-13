void handleStartWithOutRejoin(Games *games, json gameData, User user, UserList *userList)
{
    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();

    cout << gameID;
    cout << gamePin;

    json jsonMessage;
    jsonMessage["type"] = "startWatingForPlayer";

    if (!games->gamesList.empty() && gameID >= 0 && gameID < static_cast<int>(games->gamesList.size()) && games->gamesList[gameID].gamePin == gamePin && games->gamesList[gameID].gameStatus == "notWaitingForPlayers")
    {
        jsonMessage["status"] = "success";                         // send success
        games->gamesList[gameID].gameStatus = "waitingForPlayers"; // set status
        games->gamesList[gameID].gameOwnerID = user.userID;        // set game owner
        games->gamesList[gameID].users.push_back(user);
    }
    else if (!games->gamesList.empty() && (games->gamesList[gameID].gameStatus == "waitingForPlayers" ||
                                           games->gamesList[gameID].gameStatus == "started"))
    {
        jsonMessage["status"] = "waiting";
    }
    else
    {
        jsonMessage["status"] = "fail";
    }

    sendComunicate(user, jsonMessage, userList);
}

void handleStartWithRejoin(Games *games, json gameData, User user, UserList *userList)
{
    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();

    cout << gameID;
    cout << gamePin;

    json jsonMessage;
    jsonMessage["type"] = "reJoinOwner";

    if (!games->gamesList.empty() && gameID >= 0 && gameID < static_cast<int>(games->gamesList.size()) && games->gamesList[gameID].gamePin == gamePin)
    {
        for (User &userFind : games->gamesList[gameID].users)
        {

            if (userFind.userID == -1 && userFind.nickname == "")
            {
                userFind.userID = user.userID;
                break;
            }
        }

        if (games->gamesList[gameID].gameStatus == "started")
        {
            json question = questionsToJson(&games->gamesList[gameID]);
            question["type"] = "questionOwner";
            sendComunicate(user, question, userList);
        }
        else
        {
            json playersJson;
            for (User &players : games->gamesList[gameID].users)
            {

                if (players.userID != games->gamesList[gameID].gameOwnerID)
                {
                    playersJson["type"] = "playerJoin";
                    playersJson["player"] = players.nickname;

                    sendComunicate(user, playersJson, userList);
                }
            }
        }

        jsonMessage["status"] = games->gamesList[gameID].gameStatus; // send success

        games->gamesList[gameID].gameOwnerID = user.userID; // set game owner
    }
    else
    {
        jsonMessage["status"] = "fail";
    }

    sendComunicate(user, jsonMessage, userList);
}
