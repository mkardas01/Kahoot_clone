#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctime>
#include <algorithm>
#include <poll.h>
#include <fcntl.h>
#include "functions/json.hpp"

using namespace std;
using json = nlohmann::json;

#define PORT 41119
#define BUFFER_SIZE 1024
#define SERVER_ADRESS INADDR_ANY
#define TIME_TO_RESPOND 20

#include "functions/data_structurs.hpp"
#include "functions/send_data_rank.hpp"
#include "functions/print_games.hpp"
#include "functions/game_create.hpp"
#include "functions/question_to_json.hpp"
#include "functions/send_questions_or_endofgame.hpp"
#include "functions/start_game_send_player_to_lobby.hpp"
#include "functions/check_if_user_in_game.hpp"
#include "functions/check_nickname_availability.hpp"
#include "functions/player_join_rejoin_game.hpp"
#include "functions/owner_rejon.hpp"
#include "functions/owner_start_lobby.hpp"
#include "functions/send_players_answer_to_owner.hpp"
#include "functions/menage_points.hpp"
#include "functions/handle_connection.hpp"
#include "functions/handle_data.hpp"
#include "functions/check_if_next_question.hpp"


int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    Games games;
    UserList userList;

    games.generateDummyData(2); // Put sample game with id 0 and 1, gamepin is 123

    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // Create server socket

    if (server_socket == -1) // Handle errors
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = SERVER_ADRESS;

    // Bind the socket
    if (::bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Set socket to non blocking 
    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    // Listen for incoming connections
    if (listen(server_socket, 10) == -1)
    {
        perror("Error listening");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

 
    while (1) // Handle acctions
    {
        try{
            acceptClient(&userList, server_socket);

            readData(&games, &userList);

            checkIfSendNextQuestion(&games, &userList);

            //printGames(games, &userList);
            
        }catch(...){
            cerr << "Niezidentyfikowany wyjatek" << std::endl;
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
