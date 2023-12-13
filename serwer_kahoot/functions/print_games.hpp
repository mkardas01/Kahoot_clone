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
