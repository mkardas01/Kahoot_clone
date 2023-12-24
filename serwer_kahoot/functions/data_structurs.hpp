struct User
{
    string nickname;
    int userID;
    int client_socket;
    int points;

    User()
        : nickname(""), points(0){

                     };
};

struct UserList
{
    vector<pollfd> eventListener;
    vector<pollfd> sendListener;
    vector<User> users;
    vector<string> buffer;
};

struct Question
{
    string question;
    string answers[4];
    int indexOfCorrectAnswer;
};

struct GameDetails
{
    int gameID;
    int gamePin;
    int currentQuestion;
    int questionsNumber;
    int gameOwnerID;
    std::chrono::steady_clock::time_point startTime;
    string gameStatus;          // notWaitingForPlayers, waitingForPlayers, started
    vector<Question> questions; // game questions
    vector<User> users;         // game users

    GameDetails()
        : currentQuestion(0), questionsNumber(0), gameOwnerID(-2), gameStatus("notWaitingForPlayers"){

                                                                                      };
};

struct Games
{
    vector<GameDetails> gamesList; // zawiera gamedetails

    void generateDummyData(int numberOfGames)
    {
        for (int i = 0; i < numberOfGames; ++i)
        {
            GameDetails game;
            game.gameID = i;
            game.gamePin = 123;       // Losowy PIN do gry
            game.questionsNumber = 4; // Losowa liczba pytań

            // Generowanie pytań dla gry
            for (int j = 0; j < game.questionsNumber; ++j)
            {
                Question q;
                q.question = "Question " + std::to_string(j + 1);
                for (int k = 0; k < 4; ++k)
                {
                    q.answers[k] = "Answer " + std::to_string(j + 1);
                }
                q.indexOfCorrectAnswer = 0;
                game.questions.push_back(q);
            }

            gamesList.push_back(game);
        }
    }
};