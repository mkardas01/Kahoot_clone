void createGame(Games *games, json gameData, User user, UserList *userList)
{

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    GameDetails newGame; // Create new game

    newGame.gameID = games->gamesList.size();                         // Set gameid
    newGame.gamePin = rand() % 9000 + 1000;                           // Generate gamepin
    newGame.questionsNumber = gameData["questionsNumber"].get<int>(); // Set question number

    for (const auto &questionData : gameData["questions"])
    {
        Question question;
        question.question = questionData["question"].get<string>();

        for (int i = 0; i < 4; ++i)
        {
            question.answers[i] = questionData["answers"][i].get<string>();
        }

        question.indexOfCorrectAnswer = (int)questionData["indexOfCorrectAnswer"].get<int>();
        newGame.questions.push_back(question); // Set questions
    }

    cout << "Created game with ID: " << newGame.gameID << endl;

    // Add new game to gameList
    games->gamesList.push_back(newGame);

    json jsonMessage;
    jsonMessage["type"] = "createGame";
    jsonMessage["status"] = "success";
    jsonMessage["gameID"] = newGame.gameID;
    jsonMessage["gamePin"] = newGame.gamePin; // Create json respond 

    sendComunicate(user, jsonMessage, userList); // Send json respond to the creator 
}
