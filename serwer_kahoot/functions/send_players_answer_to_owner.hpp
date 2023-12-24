void sendUserAnswerToOwner(Games *games, json gameData, User user, int gameID, UserList *userList) // Send user answer to owner
{

    cout << "Found game with matching ID. answer to owner" << endl;

    // Iterate through users in game
    // for (const User &owner : games->gamesList[gameID].users)
    // {
        //cout << "Checking user in game with ID: " << owner.userID << endl;
        if (games->gamesList[gameID].gameOwnerID != -1) // Find owner
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

            sendComunicate(userList->users[games->gamesList[gameID].gameOwnerID], jsonMessage, userList); // Send nickname to owner

            cout << "Sending user ID " << user.userID << " to game owner with ID " << games->gamesList[gameID].gameOwnerID << endl;
            return;
        }
    //}
    cout << "Game owner not found in users list for the game." << endl;
}