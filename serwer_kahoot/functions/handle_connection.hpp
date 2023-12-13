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
