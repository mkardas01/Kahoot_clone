cmake_minimum_required(VERSION 3.14)
project(NazwaTwojegoProjektu)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


# Dodaj pliki źródłowe do kompilacji
add_executable(KahootSerwer
    functions/data_structurs.cpp
    functions/send_data_rank.cpp
    functions/print_games.cpp
    functions/game_create.cpp
    functions/question_to_json.cpp
    functions/send_questions_or_endofgame.cpp
    functions/start_game_send_player_to_lobby.cpp
    functions/check_if_user_in_game.cpp
    functions/check_nickname_availability.cpp
    functions/player_join_rejoin_game.cpp
    functions/owner_rejon.cpp
    functions/owner_start_lobby.cpp
    functions/send_players_answer_to_owner.cpp
    functions/menage_points.cpp
    functions/handle_connection.cpp
    functions/handle_data.cpp
    functions/check_if_next_question.cpp
)

# Tutaj możesz dodać inne instrukcje cmake, takie jak znalezienie dodatkowych bibliotek, ustawienie flag kompilacji, itp.
