void checkIfSendNextQuestion(Games *games, UserList *userList)
{

    for (const GameDetails &game : games->gamesList)
    {

        if (game.gameStatus == "started")
        {
            // cout << "Checking game with ID: " << game.gameID << endl;
            auto currentTime = std::chrono::steady_clock::now();
            auto timeSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(currentTime - game.startTime);

            auto currentGameTime = timeSinceEpoch.count();

            // cout << "Game ID: " << game.gameID << ", Time elapsed: " << currentGameTime << " seconds" << endl;

            // cout<<currentGameTime<<endl;

            if (currentGameTime >= TIME_TO_RESPOND + 10)
            {

                games->gamesList[game.gameID].currentQuestion++;

                if (game.questionsNumber <= games->gamesList[game.gameID].currentQuestion)
                { // stop game if no more questions

                    cout << "usuwam graczy z gry";
                    sendPointsSummary(games, userList, game.gameID);
                    sendQuestionsOrEndOfGame(games, game.gameID, true, userList);
                    games->gamesList[game.gameID].gameStatus = "notWaitingForPlayers";
                    games->gamesList[game.gameID].currentQuestion = 0;
                    games->gamesList[game.gameID].gameOwnerID = -2;
                    games->gamesList[game.gameID].users.clear();

                }
                else
                {
                    sendPointsSummary(games, userList, game.gameID);
                    sendQuestionsOrEndOfGame(games, game.gameID, false, userList); // send question
                }
            }
        }
    }
}
