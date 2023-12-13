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
