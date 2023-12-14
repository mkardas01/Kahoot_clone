#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <thread>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctime>
#include <algorithm>
#include <poll.h>
#include <fcntl.h>
#include "functions/json.hpp"

using namespace std;
using json = nlohmann::json;

#define PORT 41119
#define BUFFER_SIZE 1024
#define SERVER_ADRESS INADDR_ANY
#define TIME_TO_RESPOND 20

#include "functions/data_structurs.hpp"

void printGames(const Games &games, UserList *userList)
{
    cout<<endl;
    cout<<"-------"<<endl;
    cout << "Gracze z gier" << endl;

    for (const auto &user : games.gamesList[1].users)
    {

        std::cout << "user: " << user.userID << " " << user.nickname << std::endl;
    }

    cout << "Owner: " << games.gamesList[1].gameOwnerID << endl;

    cout << "Gracze z userlist" << endl;

    for (User &userListc : userList->users)
    {
        cout << userListc.userID << " " << userListc.client_socket << " " << userListc.nickname << endl;
        cout << userList->buffer[userListc.userID] << " " << userList->eventListener[userListc.userID].fd << " " << userList->sendListener[userListc.userID].fd<<endl;
    }
}

void sendComunicate(User user, json communicate, UserList *userList)
{
    if (user.userID != -1)
    {
        std::string message = communicate.dump() + "\n";
        const char *dataToSend = message.c_str();
        size_t dataSize = strlen(dataToSend);

        int poll_users = poll(&userList->sendListener[user.userID], 1, 0);

        if (poll_users > 0 && (userList->sendListener[user.userID].revents & POLLOUT))
        {
            ssize_t sentBytes = send(userList->sendListener[user.userID].fd, dataToSend, dataSize, 0);

            if (sentBytes == -1)
            {
                perror("Error sending data");
            }
        }
    }
}

void sendPointsSummary(Games *games, UserList *userList, int gameID)
{
    // Find the game with the given ID
    GameDetails &game = games->gamesList[gameID]; // Use a reference to modify the original game

    // Sort users based on points in descending order
    std::sort(game.users.begin(), game.users.end(), [](const User &a, const User &b)
              { return a.points > b.points; });

    // Create a JSON message for points summary
    json jsonMessage;
    jsonMessage["type"] = "pointsSummary";
    jsonMessage["users"] = json::array();

    // Iterate through sorted users and add them to the message
    for (const User &user : game.users)
    {
        if (user.nickname != "")
        {
            json userObj;
            userObj["nickname"] = user.nickname;
            userObj["points"] = user.points;

            jsonMessage["users"].push_back(userObj);
        }
    }

    // Send the points summary to each player
    for (const User &gameUser : game.users)
    {
        sendComunicate(gameUser, jsonMessage, userList);
    }
}

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

void sendPlayerToWaitingList(Games *games, User user, int gameID, UserList *userList)
{
    cout << "wysylam gracza";
    GameDetails game = games->gamesList[gameID];

    cout << "Found game with matching ID. waiting list" << endl;

    // Find the game owner
    for (const User &gameUser : game.users)
    {
        cout << "Checking user in game with ID: " << gameUser.userID << endl;
        if (gameUser.userID == game.gameOwnerID && user.userID != game.gameOwnerID)
        {
            cout << "Found game owner with ID: " << game.gameOwnerID << endl;

            json jsonMessage;
            jsonMessage["type"] = "playerJoin";
            jsonMessage["player"] = user.nickname;

            sendComunicate(gameUser, jsonMessage, userList);

            cout << "Sending user ID " << user.userID << " to game owner with ID " << game.gameOwnerID << endl;
            // You can implement the actual sending logic here, for example, using sockets
            return;
        }
    }
    cout << "Game owner not found in users list for the game." << endl;

    cout << "Game not found for user with ID " << user.userID << endl;
}

void startGame(Games *games, json gameData, User user, UserList *userList)
{
    json jsonMessage;
    jsonMessage["type"] = "startGame";

    int gameID = gameData["gameID"].get<int>();

    jsonMessage["status"] = "started";

    games->gamesList[gameID].gameStatus = "started";

    sendComunicate(user, jsonMessage, userList); // send respond to owner
    sendQuestionsOrEndOfGame(games, gameID, false, userList);
}

bool userNotInAnyGame(Games *games, User user)
{

    for (GameDetails &game : games->gamesList)
    {

        for (User &userGame : game.users)
        {
            if (userGame.userID == user.userID)
                return false;
        }
    }

    return true;
}

string nickNameStatus(Games *games, int gameID, string &gameNickname)
{
    // Check if the gameID is valid
    if (gameID < 0 || gameID >= static_cast<int>(games->gamesList.size()))
    {
        // Handle invalid gameID, e.g., throw an exception or return an error code
        return "error"; // error
    }

    // Iterate over the users in the specified game
    for (const User &user : games->gamesList[gameID].users)
    {
        if (user.nickname == gameNickname)
        {

            if (user.userID != -1)
                return "unavailable"; // cant rejoin user is connected to this nickname
            else
                return "rejoin"; // can rejoin user is not connectd to this nickname
        }
    }

    // The nickname is available
    return "available";
}

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

void startWatingForPlayer(Games *games, json gameData, User user, UserList *userList)
{
    if (userNotInAnyGame(games, user))
    {
        int gameID = gameData["gameID"].get<int>();

        if (games->gamesList[gameID].gameOwnerID != -1)
        {
            handleStartWithOutRejoin(games, gameData, user, userList);
            cout << " wlasciciela" << games->gamesList[gameID].gameOwnerID << endl;
        }
        else
            handleStartWithRejoin(games, gameData, user, userList);
    }
    else
    {
        json jsonMessage;
        jsonMessage["type"] = "startWatingForPlayer";
        jsonMessage["status"] = "userAlreadyInGame";

        sendComunicate(user, jsonMessage, userList);
    }
}

void sendUserAnswerToOwner(Games *games, json gameData, User user, int gameID, UserList *userList)
{

    cout << "Found game with matching ID. answer to owner" << endl;

    // Find the game owner
    for (const User &owner : games->gamesList[gameID].users)
    {
        cout << "Checking user in game with ID: " << owner.userID << endl;
        if (owner.userID == games->gamesList[gameID].gameOwnerID)
        {
            cout << "Found game owner with ID: " << games->gamesList[gameID].gameOwnerID << endl;

            json jsonMessage;
            jsonMessage["type"] = "userAnswer";
            jsonMessage["answerIndex"] = gameData["answer"];

            for (User &userGame : games->gamesList[gameID].users)
            {

                if (userGame.userID == user.userID)
                {
                    jsonMessage["nickname"] = userGame.nickname;
                    break;
                }
            }

            sendComunicate(owner, jsonMessage, userList);

            cout << "Sending user ID " << user.userID << " to game owner with ID " << games->gamesList[gameID].gameOwnerID << endl;
            // You can implement the actual sending logic here, for example, using sockets
            return;
        }
    }
    cout << "Game owner not found in users list for the game." << endl;
}

void putPointsToUser(Games *games, json gameData, User user, UserList *userList)
{

    int gameID = gameData["gameID"].get<int>();

    if (games->gamesList[gameID].questions[games->gamesList[gameID].currentQuestion].indexOfCorrectAnswer == gameData["answer"])
    {

        auto currentTime = std::chrono::steady_clock::now();
        auto points = std::chrono::duration_cast<std::chrono::seconds>(currentTime - games->gamesList[gameID].startTime);


        // Now, update the user in the gamesList
        int position = 0;
        for (auto u : games->gamesList[gameID].users)
        {
            if (u.userID == user.userID)
            {
                games->gamesList[gameID].users[position].points += 30 - points.count();
                break;
            }

            position++;
        }
    }

    sendUserAnswerToOwner(games, gameData, user, gameID, userList);
}

void handleAccept(UserList* userList, User user)
{
    // Find the first available userID
    int firstAvailableUserID = 0;

    for (const auto& existingUser : userList->users)
    {
        if (existingUser.userID != -1)
        {
            firstAvailableUserID++;
        }
        else
        {
            break;
        }
    }

    user.userID = firstAvailableUserID;

    pollfd event;
    event.fd = user.client_socket;
    event.events = POLLIN;

    if (static_cast<int>(userList->eventListener.size()) <= firstAvailableUserID)
        userList->eventListener.resize(userList->eventListener.size() + 1); // Resize eventListener vector if needed

    userList->eventListener[user.userID] = event;

    event.events = POLLOUT;

    if (static_cast<int>(userList->sendListener.size()) <= firstAvailableUserID)
        userList->sendListener.resize(userList->sendListener.size() + 1); // Resize sendListener vector if needed

    userList->sendListener[user.userID] = event;

    if (static_cast<int>(userList->users.size()) <= firstAvailableUserID)
        userList->users.resize(userList->users.size() + 1); // Resize users vector if needed

    userList->users[user.userID] = user;

    // No need to increment userID here since it's already set

    printf("Assigned userID: %d\n", user.userID);
}

void acceptClient(UserList *userList, pollfd server_events)
{
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    User user;

    int server_poll = poll(&server_events, 1, 0);

    if (server_poll > 0 && (server_events.revents & POLLIN))
    {
        addr_size = sizeof(client_addr);
        user.client_socket = accept(server_events.fd, (struct sockaddr *)&client_addr, &addr_size);

        if (user.client_socket == -1)
        {
            perror("Error accepting connection");
            close(server_events.fd);
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handleAccept(userList, user);
    }
}

void disconnectClient(Games *games, UserList *userList, User user, int i)
{
    // Zamknięcie połączenia przez klienta
    cout << "Connection closed by client " << userList->users[i].userID << endl;
    close(userList->users[i].client_socket);
    close(userList->eventListener[i].fd);
    close(userList->sendListener[i].fd);
    close(user.client_socket);

    bool found = false;

    for (int p = 0; p < static_cast<int>(games->gamesList.size()); p++)
    {
        GameDetails game = games->gamesList[p];
        cout << "sprawdzam gre o id " << p << endl;

        for (int u = 0; u < static_cast<int>(game.users.size()); u++)
        {
            cout << "sprawdzam uzytkownika o id " << u << endl;

            if (game.users[u].userID == user.userID)
            {
                games->gamesList[p].users[u].userID = -1;

                if (game.users[u].userID == game.gameOwnerID)
                    games->gamesList[p].gameOwnerID = -1;

                found = true;
                break; // Przerwij wewnętrzną pętlę
            }
        }

        if (found)
        {
            break; // Przerwij zewnętrzną pętlę
        }
    }

    if (userList->users[i].userID == userList->users[userList->users.size() - 1].userID)
    {
        userList->buffer.erase(userList->buffer.begin() + i);
        userList->eventListener.erase(userList->eventListener.begin() + i);
        userList->sendListener.erase(userList->sendListener.begin() + i);
        userList->users.erase(userList->users.begin() + i);
    }
    else
    {

        User emptyUser;
        emptyUser.userID = -1;

        // Inserting emptyUser into userList
        userList->users[i] = emptyUser;
        
        // Inserting an empty string into the buffer
        userList->buffer[i] =  "";

    }
}

void handleData(Games *games, json gameData, User user, UserList *userList)
{
    if (gameData["type"].get<std::string>() == "createGame")
    {
        createGame(games, gameData, user, userList);
    }
    else if (gameData["type"].get<std::string>() == "startWatingForPlayer")
    {
        startWatingForPlayer(games, gameData, user, userList);
    }
    else if (gameData["type"].get<std::string>() == "joinGame")
    {
        joinGame(games, gameData, user, userList);
    }
    else if (gameData["type"].get<std::string>() == "startGame")
    {
        startGame(games, gameData, user, userList);
    }
    else if (gameData["type"].get<std::string>() == "answer")
    {
        putPointsToUser(games, gameData, user, userList);
    }
}

void readData(Games *games, UserList *userList)
{
    char buf[256];
    ssize_t bytesRead;
    int poll_users = poll(userList->eventListener.data(), userList->eventListener.size(), 0);
    cout<<"poll_users "<<poll_users<<endl;
    for (int i = 0; i < static_cast<int>(userList->eventListener.size()); i++)
    {
        User user = userList->users[i];
        string message;
        size_t newlinePos;
        

        if (userList->buffer.size() < userList->eventListener.size())
            userList->buffer.resize(userList->eventListener.size());

        if (user.userID != -1 && poll_users > 0 && (userList->eventListener[i].revents & POLLIN))
        {
            try
            {
                while ((bytesRead = read(userList->eventListener[i].fd, buf, sizeof(buf) - 1)))
                {
                    buf[bytesRead] = '\0'; // Ensure null-termination

                    cout << buf << endl;

                    userList->buffer[i] += buf;

                    // Check if the newline character is present in the buffer
                    newlinePos = userList->buffer[i].find("\n");

                    if (newlinePos != std::string::npos)
                    {
                        break;
                    }
                }
            }
            catch (...)
            {
                cerr << "Niezidentyfikowany wyjatek podczas odczytu" << std::endl;
            }

            if (bytesRead > 0)
            {
                try
                {
                    message = userList->buffer[i].substr(0, newlinePos);

                    userList->buffer[i].erase(0, newlinePos + 1);

                    json jsonData;

                    printf("Received data: %s\n", message.c_str());

                    try
                    {
                        jsonData = json::parse(message);
                        printf("Type: %s\n", jsonData["type"].get<std::string>().c_str());

                        handleData(games, jsonData, user, userList);

                        cout << user.userID << endl;
                    }
                    catch (...)
                    {
                        // Handle invalid JSON
                        cerr << "Niezidentyfikowany wyjatek while parsing data" << std::endl;
                    }
                }
                catch (...)
                {
                    cerr << "Niezidentyfikowany wyjatek bytes read" << std::endl;
                }
            }
            else
            {
                disconnectClient(games, userList, user, i);
            }
        }
    }
}

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

int main()
{
    Games games;
    UserList userList;
    pollfd server_events;

    games.generateDummyData(2);

    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = SERVER_ADRESS;

    // Bind the socket
    if (::bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    // Listen for incoming connections
    if (listen(server_socket, 10) == -1)
    {
        perror("Error listening");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    server_events.fd = server_socket;
    server_events.events = POLLIN;

    while (1)
    {
        try{
            acceptClient(&userList, server_events);

            readData(&games, &userList);

            checkIfSendNextQuestion(&games, &userList);

            printGames(games, &userList);
            
        }catch(...){
            cerr << "Niezidentyfikowany wyjatek" << std::endl;
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
