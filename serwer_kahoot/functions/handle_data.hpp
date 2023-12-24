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

void readData(Games *games, UserList *userList)
{
    char buf[256];
    ssize_t bytesRead;
    int timeout = 500;

    for (int i = 0; i < static_cast<int>(userList->eventListener.size()); i++)
    {
        User user = userList->users[i];
        string message;
        size_t newlinePos;

        int poll_users = poll(&userList->eventListener[i], 1, timeout);

        if (userList->buffer.size() < userList->eventListener.size())
            userList->buffer.resize(userList->eventListener.size());

        if (user.userID != -1 && poll_users > 0 && (userList->eventListener[i].revents & POLLIN))
        {
            try
            {
                bytesRead = read(userList->eventListener[i].fd, buf, sizeof(buf) - 1);

                if (bytesRead > 0)
                {
                    buf[bytesRead] = '\0'; // Ensure null-termination

                    cout << buf << endl;

                    userList->buffer[i] += buf;

                    // Check if the newline character is present in the buffer
                    newlinePos = userList->buffer[i].find("\n");

                    if (newlinePos != std::string::npos)
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
                            cerr << "Niezidentyfikowany wyjatek while parsing data" << std::endl;
                        }
                    }
                }
                else
                {
                    disconnectClient(games, userList, user, i);
                }
            }
            catch (...)
            {
                cerr << "Niezidentyfikowany wyjatek podczas odczytu" << std::endl;
            }
        }
    }
}
