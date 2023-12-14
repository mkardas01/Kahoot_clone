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
