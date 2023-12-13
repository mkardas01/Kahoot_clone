void sendQuestionsOrEndOfGame(Games *games, int gameID, bool endOfGame, UserList *userList)
{

    json jsonMessage;
    GameDetails game = games->gamesList[gameID];

    cout << "Found game with matching ID. questions" << endl;

    for (const User &gameUser : game.users) // send current question to all users except owner
    {
        if (!endOfGame)
        {
            jsonMessage = questionsToJson(&game);

            if (gameUser.userID == game.gameOwnerID)
            {
                jsonMessage["type"] = "questionOwner";
            }

            sendComunicate(gameUser, jsonMessage, userList);
        }
        else
        {
            jsonMessage["type"] = "endOfGame";
            sendComunicate(gameUser, jsonMessage, userList);
        }
    }

    games->gamesList[gameID].startTime = std::chrono::time_point_cast<std::chrono::steady_clock::duration>(std::chrono::steady_clock::now());


    return;
    cout << "Game owner not found in users list for the game." << endl;
}
