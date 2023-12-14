void checkIfSendNextQuestion(Games *games, UserList *userList) // Check if it's time to send new question to players
{

    for (const GameDetails &game : games->gamesList)
    {

        if (game.gameStatus == "started") // Check if game is started
        {

            auto currentTime = std::chrono::steady_clock::now();
            auto timeSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(currentTime - game.startTime); // Calculate time

            auto currentGameTime = timeSinceEpoch.count();

            if (currentGameTime >= TIME_TO_RESPOND + 10) // If it's time for new question
            {

                games->gamesList[game.gameID].currentQuestion++;

                if (game.questionsNumber <= games->gamesList[game.gameID].currentQuestion)
                { // stop game if no more questions

                    cout << "usuwam graczy z gry";

                    sendPointsSummary(games, userList, game.gameID); // Send summary to everyone
                    sendQuestionsOrEndOfGame(games, game.gameID, true, userList); // Send end of game to everoyne

                    games->gamesList[game.gameID].gameStatus = "notWaitingForPlayers"; // Change status, game is available to start
                    games->gamesList[game.gameID].currentQuestion = 0; // Reset currentquestion counter
                    games->gamesList[game.gameID].gameOwnerID = -2; // Game is waiting for owner
                    games->gamesList[game.gameID].users.clear(); // Delete users from game

                }
                else
                {
                    sendPointsSummary(games, userList, game.gameID); // Send rank
                    sendQuestionsOrEndOfGame(games, game.gameID, false, userList); // Send questions
                }
            }
        }
    }
}
