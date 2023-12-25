#ifndef SEND_DATA_RANK_HPP
#define SEND_DATA_RANK_HPP
void sendComunicate(User user, json communicate, UserList *userList);

void sendPointsSummary(Games *games, UserList *userList, int gameID);
#endif // SEND_DATA_RANK_HPP
