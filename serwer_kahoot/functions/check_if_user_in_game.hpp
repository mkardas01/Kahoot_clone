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
