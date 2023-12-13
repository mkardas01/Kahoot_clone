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
