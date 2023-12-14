#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
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

void printGames(const Games &games, UserList *userList) // print status of game with ID 1
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

void sendComunicate(User user, json communicate, UserList *userList) // send message to specific user
{
    if (user.userID != -1) // Checj=k if user is not disconnected 
    {
        std::string message = communicate.dump() + "\n"; // Add end of line character at the end 
        const char *dataToSend = message.c_str();
        size_t dataSize = strlen(dataToSend);

        int poll_users = poll(&userList->sendListener[user.userID], 1, 0); 

        if (poll_users > 0 && (userList->sendListener[user.userID].revents & POLLOUT)) // Check if message can be send 
        {
            ssize_t sentBytes = send(userList->sendListener[user.userID].fd, dataToSend, dataSize, 0); // Send message 

            if (sentBytes == -1) // Check for errors
            {
                perror("Error sending data");
            }
        }
    }
}

void sendPointsSummary(Games *games, UserList *userList, int gameID) // Send points summary
{
    // Find the game with the given ID
    GameDetails &game = games->gamesList[gameID]; 

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
        if (user.nickname != "") // Check if user is not a owner (owner can be showed in summary)
        {
            json userObj;
            userObj["nickname"] = user.nickname;
            userObj["points"] = user.points;

            jsonMessage["users"].push_back(userObj); // Add users
        }
    }

    // Send the points summary to each player in game
    for (const User &gameUser : game.users)
    {
        sendComunicate(gameUser, jsonMessage, userList);
    }
}

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

json questionsToJson(const GameDetails *game) // Get questions to json
{
    json jsonGame;

    jsonGame["gameID"] = game->gameID;
    jsonGame["currentQuestion"] = game->currentQuestion; 
    jsonGame["questionList"] = json::array();  // Create json message

    if (game->currentQuestion < static_cast<int>(game->questions.size()))
    {
        const auto &currentQuestion = game->questions[game->currentQuestion];

        json questionObj;

        questionObj["question"] = currentQuestion.question;
        questionObj["answers"] = json::array({currentQuestion.answers[0],
                                              currentQuestion.answers[1],
                                              currentQuestion.answers[2],
                                              currentQuestion.answers[3]}); // Push question to json message

        jsonGame["type"] = "question";
        jsonGame["questionList"] = questionObj; // Add details of the current question
    }

    return jsonGame; // Return json
}

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

void sendPlayerToWaitingList(Games *games, User user, int gameID, UserList *userList) // handle lobby 
{
    cout << "wysylam gracza";
    GameDetails game = games->gamesList[gameID];

    cout << "Found game with matching ID. waiting list" << endl;

    // Iterate through game's users
    for (const User &gameUser : game.users)
    {
        cout << "Checking user in game with ID: " << gameUser.userID << endl;
        if (gameUser.userID == game.gameOwnerID && user.userID != game.gameOwnerID) // Find owner
        {
            cout << "Found game owner with ID: " << game.gameOwnerID << endl;

            json jsonMessage;
            jsonMessage["type"] = "playerJoin";
            jsonMessage["player"] = user.nickname;

            sendComunicate(gameUser, jsonMessage, userList); // Send info about joining user to game owner

            cout << "Sending user ID " << user.userID << " to game owner with ID " << game.gameOwnerID << endl;

            return;
        }
    }
    cout << "Game owner not found in users list for the game." << endl;

    cout << "Game not found for user with ID " << user.userID << endl;
}

void startGame(Games *games, json gameData, User user, UserList *userList) // start game (stop waiting in lobby for players)
{
    json jsonMessage;
    jsonMessage["type"] = "startGame"; // Prepare json message 

    int gameID = gameData["gameID"].get<int>();

    jsonMessage["status"] = "started";

    games->gamesList[gameID].gameStatus = "started"; // Change game status

    sendComunicate(user, jsonMessage, userList); // Send respond to owner
    sendQuestionsOrEndOfGame(games, gameID, false, userList); // Send questions to players
}

bool userNotInAnyGame(Games *games, User user) // Check if user is not in any game
{

    for (GameDetails &game : games->gamesList) // Iterate through games
    {

        for (User &userGame : game.users) // Iterate through users in game 
        {
            if (userGame.userID == user.userID)
                return false;
        }
    }

    return true;
}

string nickNameStatus(Games *games, int gameID, string &gameNickname) // Check nickname status
{
    // Check if the gameID is valid
    if (gameID < 0 || gameID >= static_cast<int>(games->gamesList.size()))
    {
        // Handle invalid gameID
        return "error"; // error
    }

    // Iterate over the users in the specified game
    for (const User &user : games->gamesList[gameID].users)
    {
        if (user.nickname == gameNickname)
        {

            if (user.userID != -1)
                return "unavailable"; // Cant rejoin user is connected to this nickname
            else
                return "rejoin"; // Can rejoin, user is not connectd to this nickname
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
        games->gamesList[gameID].gameStatus == "waitingForPlayers") // Check if game is available 

    {

        status = "success";
        user.nickname = gameNickname;
        games->gamesList[gameID].users.push_back(user); // Add users to game 

        sendPlayerToWaitingList(games, user, gameID, userList); // Send user to lobby (owner see who joined the game)
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

string handleReJoin(Games *games, json gameData, User user) // handle rejoin to game (only players)
{
    int gameID = gameData["gameID"].get<int>();
    string gameNickname = gameData["nickname"].get<string>();

    for (User &userFind : games->gamesList[gameID].users) // Iterate through games 
    {
        if (userFind.nickname == gameNickname) // Find available nickname to rejoin 
        {
            userFind.userID = user.userID;
            break;
        }
    }

    return "reJoin";
}

void joinGame(Games *games, json gameData, User user, UserList *userList) // Join game (only players)
{
    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();
    string gameNickname = gameData["nickname"].get<string>();

    cout << gameID << endl;
    cout << gamePin << endl;

    json jsonMessage;
    jsonMessage["type"] = "joinGame";

    if (userNotInAnyGame(games, user)) // Check if user is not in any other game 
    {

        string checkNickNameStatus = nickNameStatus(games, gameID, gameNickname); // Check nickname status

        if (checkNickNameStatus == "unavailable")
        {
            //Nickname not available
            jsonMessage["status"] = "nickNameNotAvailable";
        }
        else if (checkNickNameStatus == "rejoin")
        {
            // Can rejoin to this nickname
            jsonMessage["status"] = handleReJoin(games, gameData, user);
        }
        else if (checkNickNameStatus == "available")
        {
            // Nickname available
            jsonMessage["status"] = handleAddingUserToGame(games, gameData, user, userList);
        }
    }
    else
    {
        jsonMessage["status"] = "userAlreadyInGame";
    }

    sendComunicate(user, jsonMessage, userList); // Send message to user
}

void handleStartWithOutRejoin(Games *games, json gameData, User user, UserList *userList) // Handle start (owner)
{
    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();

    cout << gameID;
    cout << gamePin;

    json jsonMessage;
    jsonMessage["type"] = "startWatingForPlayer";

    // Check game info
    if (!games->gamesList.empty() && gameID >= 0 && gameID < static_cast<int>(games->gamesList.size()) && games->gamesList[gameID].gamePin == gamePin && games->gamesList[gameID].gameStatus == "notWaitingForPlayers")
    {
        jsonMessage["status"] = "success";                         // Ssend success
        games->gamesList[gameID].gameStatus = "waitingForPlayers"; // Set status
        games->gamesList[gameID].gameOwnerID = user.userID;        // Set game owner
        games->gamesList[gameID].users.push_back(user);            // Push owner to users in game with empty nickname 
    }
    else if (!games->gamesList.empty() && (games->gamesList[gameID].gameStatus == "waitingForPlayers" ||
                                           games->gamesList[gameID].gameStatus == "started")) // Game is waiting for players (lobby) or is started
    {
        jsonMessage["status"] = "waiting";
    }
    else
    {
        jsonMessage["status"] = "fail"; // Fail game is not existing
    }

    sendComunicate(user, jsonMessage, userList); // Send respond
}

void handleStartWithRejoin(Games *games, json gameData, User user, UserList *userList) // Handle start with rejoin (owner)
{
    int gameID = gameData["gameID"].get<int>();
    int gamePin = gameData["gamePin"].get<int>();

    cout << gameID;
    cout << gamePin;

    json jsonMessage;
    jsonMessage["type"] = "reJoinOwner"; // Prepare json type respond

    if (!games->gamesList.empty() && gameID >= 0 && gameID < static_cast<int>(games->gamesList.size()) && games->gamesList[gameID].gamePin == gamePin)
    {
        for (User &userFind : games->gamesList[gameID].users) // Iterate through users in this game 
        {

            if (userFind.userID == -1 && userFind.nickname == "") // Find owner
            {
                userFind.userID = user.userID; // Set owner
                break;
            }
        }

        if (games->gamesList[gameID].gameStatus == "started") // If game is already on
        {
            json question = questionsToJson(&games->gamesList[gameID]);
            question["type"] = "questionOwner";
            sendComunicate(user, question, userList); // Send current question to owner
        }
        else // Game is in lobby
        {
            json playersJson;
            for (User &players : games->gamesList[gameID].users) // Iterate through users in game
            {

                if (players.userID != games->gamesList[gameID].gameOwnerID) // Check is user is no a owner
                {
                    playersJson["type"] = "playerJoin";
                    playersJson["player"] = players.nickname;

                    sendComunicate(user, playersJson, userList); // Send user nickname to owner, to let him see nicknames in lobby
                }
            }
        }

        jsonMessage["status"] = games->gamesList[gameID].gameStatus; // Send success

        games->gamesList[gameID].gameOwnerID = user.userID; // Set game owner
    }
    else
    {
        jsonMessage["status"] = "fail"; // Game doesn't exists
    }

    sendComunicate(user, jsonMessage, userList); // Send communicate
}

void startWatingForPlayer(Games *games, json gameData, User user, UserList *userList) // Start lobby
{
    if (userNotInAnyGame(games, user)) // Check if user how want to start lobby is not in any other game 
    {
        int gameID = gameData["gameID"].get<int>();

        if (games->gamesList[gameID].gameOwnerID != -1) // Check if game hadn't had any previous owner
        {
            handleStartWithOutRejoin(games, gameData, user, userList); // Handle starting lobby without rejoining
            cout << " wlasciciela" << games->gamesList[gameID].gameOwnerID << endl;
        }
        else // Game had previous owner
            handleStartWithRejoin(games, gameData, user, userList); // Handle starting lobby with rejoining
    }
    else // User is in other game send communicate
    {
        json jsonMessage;
        jsonMessage["type"] = "startWatingForPlayer";
        jsonMessage["status"] = "userAlreadyInGame";

        sendComunicate(user, jsonMessage, userList);
    }
}

void sendUserAnswerToOwner(Games *games, json gameData, User user, int gameID, UserList *userList) // Send user answer to owner
{

    cout << "Found game with matching ID. answer to owner" << endl;

    // Iterate through users in game
    for (const User &owner : games->gamesList[gameID].users)
    {
        cout << "Checking user in game with ID: " << owner.userID << endl;
        if (owner.userID == games->gamesList[gameID].gameOwnerID) // Find owner
        {
            cout << "Found game owner with ID: " << games->gamesList[gameID].gameOwnerID << endl;

            json jsonMessage;
            jsonMessage["type"] = "userAnswer";
            jsonMessage["answerIndex"] = gameData["answer"]; // Prepare json message

            for (User &userGame : games->gamesList[gameID].users) // Find user in game
            {

                if (userGame.userID == user.userID)
                {
                    jsonMessage["nickname"] = userGame.nickname; // Set his nickname
                    break;
                }
            }

            sendComunicate(owner, jsonMessage, userList); // Send nickname to owner

            cout << "Sending user ID " << user.userID << " to game owner with ID " << games->gamesList[gameID].gameOwnerID << endl;
            // You can implement the actual sending logic here, for example, using sockets
            return;
        }
    }
    cout << "Game owner not found in users list for the game." << endl;
}

void putPointsToUser(Games *games, json gameData, User user, UserList *userList) // Add points to user
{

    int gameID = gameData["gameID"].get<int>();

    if (games->gamesList[gameID].questions[games->gamesList[gameID].currentQuestion].indexOfCorrectAnswer == gameData["answer"]) // Check if answer is correct
    {

        auto currentTime = std::chrono::steady_clock::now();
        auto points = std::chrono::duration_cast<std::chrono::seconds>(currentTime - games->gamesList[gameID].startTime); // Calculate tiem


        // Update the user in the gamesList
        int position = 0;
        for (auto u : games->gamesList[gameID].users) // Iterate through users
        {
            if (u.userID == user.userID)
            {
                games->gamesList[gameID].users[position].points += 30 - points.count(); // Add points 
                break;
            }

            position++;
        }
    }

    sendUserAnswerToOwner(games, gameData, user, gameID, userList); // Send owner info about user's answer
}

void handleAccept(UserList* userList, User user) // Handle accepting new connection 
{
    // Find the first available userID
    int firstAvailableUserID = 0;

    for (const auto& existingUser : userList->users)
    {
        if (existingUser.userID != -1) // userID = -1 means user is disconnected
        {
            firstAvailableUserID++;
        }
        else
        {
            break;
        }
    }

    //Create pollfd 

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

    userList->users[user.userID] = user; // Add user to userList

    printf("Assigned userID: %d\n", user.userID);
}

void acceptClient(UserList *userList, pollfd server_events) // Accept new connection
{
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    User user;

    int server_poll = poll(&server_events, 1, 0);

    if (server_poll > 0 && (server_events.revents & POLLIN)) // If new event at server descriptor 
    {
        addr_size = sizeof(client_addr);
        user.client_socket = accept(server_events.fd, (struct sockaddr *)&client_addr, &addr_size); // Accept connection

        if (user.client_socket == -1)
        {
            perror("Error accepting connection");
            close(server_events.fd);
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handleAccept(userList, user); // Handle accept
    }
}

void disconnectClient(Games *games, UserList *userList, User user, int i) // Disconnect user
{
    // Zamknięcie połączenia przez klienta
    cout << "Connection closed by client " << userList->users[i].userID << endl;

    close(userList->users[i].client_socket); // Close any exitisng descriptor 
    close(userList->eventListener[i].fd);
    close(userList->sendListener[i].fd);
    close(user.client_socket);

    bool found = false;

    for (int p = 0; p < static_cast<int>(games->gamesList.size()); p++) // Find disconnected user in game
    {
        GameDetails game = games->gamesList[p];
        cout << "sprawdzam gre o id " << p << endl;

        for (int u = 0; u < static_cast<int>(game.users.size()); u++) // Find user in game 
        {
            cout << "sprawdzam uzytkownika o id " << u << endl;

            if (game.users[u].userID == user.userID) // Set his userid to -1 (id -1 means disconnected)
            {
                games->gamesList[p].users[u].userID = -1;

                if (game.users[u].userID == game.gameOwnerID) // Check if user was a gameowner if yes set gameownerid to -1
                    games->gamesList[p].gameOwnerID = -1;

                found = true;
                break; 
            }
        }

        if (found)
        {
            break;
        }
    }

    if (userList->users[i].userID == userList->users[userList->users.size() - 1].userID) // If user in userlist was at the end of userlist vector, delete it
    {
        userList->buffer.erase(userList->buffer.begin() + i);
        userList->eventListener.erase(userList->eventListener.begin() + i);
        userList->sendListener.erase(userList->sendListener.begin() + i);
        userList->users.erase(userList->users.begin() + i);
    }
    else // Otherwise put empoty user with id -1
    {

        User emptyUser;
        emptyUser.userID = -1;

        // Inserting emptyUser into userList
        userList->users[i] = emptyUser;
        
        // Inserting an empty string into the buffer
        userList->buffer[i] =  "";

    }
}

void handleData(Games *games, json gameData, User user, UserList *userList) // Handle data from users
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

void readData(Games *games, UserList *userList) // Read data
{
    char buf[256];
    ssize_t bytesRead;

    int poll_users = poll(userList->eventListener.data(), userList->eventListener.size(), 0);

    cout<<"poll_users "<<poll_users<<endl;

    for (int i = 0; i < static_cast<int>(userList->eventListener.size()); i++) // Iterate through all users
    {
        User user = userList->users[i];
        string message;
        size_t newlinePos;
        

        if (userList->buffer.size() < userList->eventListener.size())
            userList->buffer.resize(userList->eventListener.size()); // Resize buffor vector if we have more users 

        if (user.userID != -1 && poll_users > 0 && (userList->eventListener[i].revents & POLLIN)) // Check if users is not disconnected (id != -1) and if user sent something 
        {
            try
            {
                while ((bytesRead = read(userList->eventListener[i].fd, buf, sizeof(buf) - 1))) // Read data 
                {
                    buf[bytesRead] = '\0'; // Ensure null-termination

                    cout << buf << endl;

                    userList->buffer[i] += buf;

                    // Check if the newline character is present in the buffer
                    newlinePos = userList->buffer[i].find("\n");

                    if (newlinePos != std::string::npos) // If newline character is present break
                    {
                        break;
                    }
                }
            }
            catch (...)
            {
                cerr << "Niezidentyfikowany wyjatek podczas odczytu" << std::endl;
            }

            if (bytesRead > 0) // If user didn't disconnect
            {
                try
                {
                    message = userList->buffer[i].substr(0, newlinePos);

                    userList->buffer[i].erase(0, newlinePos + 1); // Cut data from buffer to newline character

                    json jsonData;

                    printf("Received data: %s\n", message.c_str());

                    try
                    {
                        jsonData = json::parse(message); // parse to json
                        printf("Type: %s\n", jsonData["type"].get<std::string>().c_str());

                        handleData(games, jsonData, user, userList); // Handle data

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
            else // If user disconnect
            {
                disconnectClient(games, userList, user, i); // Handle disconnect
            }
        }
    }
}

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

int main()
{
    Games games;
    UserList userList;
    pollfd server_events;

    games.generateDummyData(2); // Put sample game with id 0 and 1, gamepin is 123

    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // Create server socket

    if (server_socket == -1) // Handle errors
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

    // Set socket to non blocking 
    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    // Listen for incoming connections
    if (listen(server_socket, 10) == -1)
    {
        perror("Error listening");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Create server event listener
    server_events.fd = server_socket;
    server_events.events = POLLIN;

    while (1) // Handle acctions
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
