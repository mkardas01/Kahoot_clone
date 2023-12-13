json questionsToJson(const GameDetails *game)
{
    json jsonGame;

    jsonGame["gameID"] = game->gameID;
    jsonGame["currentQuestion"] = game->currentQuestion; // Add currentQuestion directly
    jsonGame["questionList"] = json::array();            // Stwórz tablicę JSON

    if (game->currentQuestion < static_cast<int>(game->questions.size()))
    {
        const auto &currentQuestion = game->questions[game->currentQuestion];

        json questionObj;

        questionObj["question"] = currentQuestion.question;
        questionObj["answers"] = json::array({currentQuestion.answers[0],
                                              currentQuestion.answers[1],
                                              currentQuestion.answers[2],
                                              currentQuestion.answers[3]});

        jsonGame["type"] = "question";
        jsonGame["questionList"] = questionObj; // Add details of the current question
    }

    return jsonGame;
}
