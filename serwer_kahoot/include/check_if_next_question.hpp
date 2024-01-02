#ifndef CHECK_IF_NEXT_QUESTION_HPP
#define CHECK_IF_NEXT_QUESTION_HPP

void checkIfSendNextQuestion(Games *games, UserList *userList, std::vector<GameDetails> *startedGamesList, MessageQueue* messageQueue); // Check if it's time to send new question to players
#endif // CHECK_IF_NEXT_QUESTION_HPP
