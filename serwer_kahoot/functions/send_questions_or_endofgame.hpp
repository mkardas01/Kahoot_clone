void sendQuestionsOrEndOfGame(Games *games, int gameID, bool endOfGame, UserList *userList) // Handle questions and end of game
{

    json jsonMessage;
    GameDetails game = games->gamesList[gameID];

    cout << "Found game with matching ID. questions" << endl;

    for (const User &gameUser : game.users) // Send current question or end of game to all users 
    {
        if (!endOfGame) // If game is still on
        {
            jsonMessage = questionsToJson(&game); // Get question

            if (gameUser.userID == game.gameOwnerID) // For owner change type of message
            {
                jsonMessage["type"] = "questionOwner";
            }

            sendComunicate(gameUser, jsonMessage, userList); // Send it 
        }
        else // If game is not still on
        {
            jsonMessage["type"] = "endOfGame"; 
            sendComunicate(gameUser, jsonMessage, userList); // Send comunicate about ending of game
        }
    }

    games->gamesList[gameID].startTime = std::chrono::time_point_cast<std::chrono::steady_clock::duration>(std::chrono::steady_clock::now()); // Start timer of current question

}
