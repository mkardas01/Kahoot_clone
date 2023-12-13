void createGame(Games *games, json gameData, User user, UserList *userList)
{

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    GameDetails newGame; // create new game

    newGame.gameID = games->gamesList.size();                         // set gameid
    newGame.gamePin = rand() % 9000 + 1000;                           // set gamepin
    newGame.questionsNumber = gameData["questionsNumber"].get<int>(); // set question number

    for (const auto &questionData : gameData["questions"])
    {
        Question question;
        question.question = questionData["question"].get<string>();

        for (int i = 0; i < 4; ++i)
        {
            question.answers[i] = questionData["answers"][i].get<string>();
        }

        question.indexOfCorrectAnswer = (int)questionData["indexOfCorrectAnswer"].get<int>();
        newGame.questions.push_back(question); // set questions
    }

    cout << "Created game with ID: " << newGame.gameID << endl;

    // Dodaj nową grę do listy gier
    games->gamesList.push_back(newGame);

    json jsonMessage;
    jsonMessage["type"] = "createGame";
    jsonMessage["status"] = "success";
    jsonMessage["gameID"] = newGame.gameID;
    jsonMessage["gamePin"] = newGame.gamePin;

    sendComunicate(user, jsonMessage, userList);
}
