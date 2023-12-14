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