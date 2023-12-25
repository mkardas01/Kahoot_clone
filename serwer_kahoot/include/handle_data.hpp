#ifndef HANDLE_DATA_HPP
#define HANDLE_DATA_HPP

void handleData(Games *games, json gameData, User user, UserList *userList, std::vector<GameDetails> *startedGamesList);

void readData(Games *games, UserList *userList, std::vector<GameDetails>* startedGamesList);
#endif //HANDLE_DATA_HPP
